//
// Created by ChenXin on 2022/9/29.
//

#include "imageio.h"

#include <tinyexr.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#include <core/logger.h>

namespace gl_render {

    namespace impl {

        void save_hdr_image(path output_path, const float *pixels, uint2 resolution, int channel) noexcept {
            auto size = int2{resolution.x, resolution.y};

            if (!is_hdr_image(output_path)) [[unlikely]] {
                GL_RENDER_WARNING_WITH_LOCATION(
                        "Unexpected output file extension. "
                        "Changing to '.exr'.");
                output_path.replace_extension(".exr");
            }

            if (output_path.extension() == ".exr") {
                const char *err = nullptr;
                SaveEXR(reinterpret_cast<const float *>(pixels),
                        size.x, size.y, channel, false, output_path.string().c_str(), &err);
                if (err != nullptr) [[unlikely]] {
                    GL_RENDER_WARNING_WITH_LOCATION(
                            "Failed to save image to '{}', err: {}.",
                            output_path.string(), err);
                }
            } else if (output_path.extension() == ".hdr") {
                int ret = stbi_write_hdr(output_path.string().c_str(), size.x, size.y, channel,
                                         reinterpret_cast<const float *>(pixels));
                if (ret == 0) [[unlikely]] {
                    GL_RENDER_WARNING_WITH_LOCATION(
                            "Failed to save image to '{}'.",
                            output_path.string());
                }
            }
            GL_RENDER_INFO("Saved image to '{}'. resolution = {}, channel = {}", output_path.string(), to_string(resolution), channel);
        }

        void save_ldr_image(path output_path, const uchar *pixels, uint2 resolution, int channel) noexcept {
            auto size = int2{resolution.x, resolution.y};

            if (!is_ldr_image(output_path)) [[unlikely]] {
                GL_RENDER_WARNING_WITH_LOCATION(
                        "Unexpected output file extension. "
                        "Changing to '.png'.");
                output_path.replace_extension(".png");
            }

            int ret = 0;
            if (output_path.extension() == ".png") {
                ret = stbi_write_png(output_path.string().c_str(), size.x, size.y, channel, pixels, 0);
            } else if (output_path.extension() == ".bmp") {
                ret = stbi_write_bmp(output_path.string().c_str(), size.x, size.y, channel, pixels);
            } else if (output_path.extension() == ".jpg" || output_path.extension() == ".jpeg") {
                ret = stbi_write_jpg(output_path.string().c_str(), size.x, size.y, channel, pixels, 100);
            }
            if (ret == 0) [[unlikely]] {
                GL_RENDER_WARNING_WITH_LOCATION(
                        "Failed to save image to '{}'.",
                        output_path.string());
            }
            GL_RENDER_INFO("Saved image to '{}'. resolution = {}, channel = {}", output_path.string(), to_string(resolution), channel);
        }

    }

    void save_image(path output_path, const float *pixels, uint2 resolution, int channel, HDRConfig hdr_config) noexcept {
        GL_RENDER_ASSERT(channel == 3 || channel == 4, "Image saving only supports 3 or 4 channels");

        if (is_hdr_image(output_path)) {
            impl::save_hdr_image(output_path, pixels, resolution, channel);
        } else if (is_ldr_image(output_path)) {
            auto ldr_pixels = vector<uchar>(resolution.x * resolution.y * channel);
            hdr2srgb(pixels, ldr_pixels.data(), resolution, channel, hdr_config);
            impl::save_ldr_image(output_path, ldr_pixels.data(), resolution, channel);
        } else {
            GL_RENDER_WARNING_WITH_LOCATION(
                    "Unexpected output file extension. "
                    "Changing to '.exr'.");
            output_path.replace_extension(".exr");
            impl::save_hdr_image(output_path, pixels, resolution, channel);
        }
    }

    void save_image(path output_path, const uchar *pixels, uint2 resolution, int channel) noexcept {
        GL_RENDER_ASSERT(channel == 3 || channel == 4, "Image saving only supports 3 or 4 channels");

        if (!is_ldr_image(output_path)) {
            GL_RENDER_WARNING_WITH_LOCATION(
                    "Unexpected output file extension. "
                    "Changing to '.png'.");
            output_path.replace_extension(".png");
        }
        impl::save_ldr_image(output_path, pixels, resolution, channel);
    }

    void hdr2srgb(const float *hdr, uchar *srgb, uint2 resolution, int channel, HDRConfig hdr_config) noexcept {
        GL_RENDER_ASSERT(channel == 3 || channel == 4, "Image saving only supports 3 or 4 channels");

        auto inv_gamma = 1.f / hdr_config.gamma;
        for (int i = 0; i < resolution.x * resolution.y; ++i) {
            for (int j = 0; j < 3; ++j) {
                auto ldr_float = pow(1.f - std::exp(-hdr[i * channel + j] * hdr_config.exposure), inv_gamma);
                auto ldr = ldr_float * 255.f;
                srgb[i * channel + j] = static_cast<uchar>(ldr);
            }
            if (channel == 4) {
                srgb[i * channel + 3] = 255;
            }
        }
    }

    bool is_hdr_image(const path &image_path) noexcept {
        static const unordered_set<string> HDR_IMAGE_EXT = {
                ".hdr", ".exr"
        };
        return HDR_IMAGE_EXT.contains(image_path.extension().string());
    }

    bool is_ldr_image(const path &image_path) noexcept {
        static const unordered_set<string> LDR_IMAGE_EXT = {
                ".bmp", ".png", ".jpg", ".jpeg"
        };
        return LDR_IMAGE_EXT.contains(image_path.extension().string());
    }

    gl_render::vector<uchar4> load_ldr_image(const path& input_path, uint2 &resolution) noexcept {
        int w, h, d;
        auto data = stbi_load(input_path.string().c_str(), &w, &h, &d, 4);
        GL_RENDER_ASSERT(data != nullptr, "Failed to load texture: {}", input_path.string());
        gl_render::vector<uchar4> image(w * h);
        std::memmove(image.data(), data, w * h * 4 * sizeof(uchar));
        stbi_image_free(data);
        resolution = uint2{w, h};
        return std::move(image);
    }

    gl_render::vector<float4> load_hdr_image(const path& input_path, uint2 &resolution) noexcept {
        int w, h;
        gl_render::vector<float4> image;
        if (input_path.extension() == ".exr") {
//            GL_RENDER_ASSERT(IsEXR(input_path.string().c_str()), "Invalid HDR texture: {}", input_path.string());
            const char *err = nullptr;
            float4 *data = nullptr;
            auto ret = LoadEXR(reinterpret_cast<float **>(&data), &w, &h, input_path.string().c_str(), &err);
            if (ret != TINYEXR_SUCCESS && err != nullptr) {
                GL_RENDER_ERROR("Failed to load texture: {}, {}", input_path.string(), err);
                FreeEXRErrorMessage(err);
            }
            image.reserve(w * h);
            std::memmove(image.data(), data, w * h * 4 * sizeof(float));
            free(data);
        } else {
            int d;
            auto data = stbi_loadf(input_path.string().c_str(), &w, &h, &d, 4);
            GL_RENDER_ASSERT(data != nullptr, "Failed to load texture: {}", input_path.string());
            image.reserve(w * h);
            std::memmove(image.data(), data, w * h * 4 * sizeof(float));
            stbi_image_free(data);
        }
        resolution = uint2{w, h};
        return std::move(image);
    }

}