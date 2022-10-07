//
// Created by ChenXin on 2022/9/29.
//

#include "imageio.h"

#include <tinyexr.h>
#include <stb/stb_image_write.h>

#include <core/logger.h>

namespace gl_render {

    const unordered_set<string> HDR_IMAGE_EXT = {
            ".hdr", ".exr"
    };
    const unordered_set<string> LDR_IMAGE_EXT = {
            ".bmp", ".png", ".jpg", ".jpeg"
    };

    namespace impl {

        void save_hdr_image(path output_path, const float *pixels, uint2 resolution, int channel) noexcept {
            auto size = int2{resolution.x, resolution.y};

            if (output_path.extension() != ".exr" && output_path.extension() != ".hdr") [[unlikely]] {
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
                            "Failed to save image to '{}'.",
                            output_path.string());
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

            if (output_path.extension() != ".bmp" && output_path.extension() != ".png") [[unlikely]] {
                GL_RENDER_WARNING_WITH_LOCATION(
                        "Unexpected output file extension. "
                        "Changing to '.png'.");
                output_path.replace_extension(".png");
            }

            int ret = 1;
            if (output_path.extension() == ".png") {
                ret = stbi_write_png(output_path.string().c_str(), size.x, size.y, channel, pixels, 0);
            } else if (output_path.extension() == ".bmp") {
                ret = stbi_write_bmp(output_path.string().c_str(), size.x, size.y, channel, pixels);
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

        if (HDR_IMAGE_EXT.contains(output_path.extension().string())) {
            impl::save_hdr_image(output_path, pixels, resolution, channel);
        } else if (LDR_IMAGE_EXT.contains(output_path.extension().string())) {
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

        if (LDR_IMAGE_EXT.contains(output_path.extension().string())) {
            impl::save_ldr_image(output_path, pixels, resolution, channel);
        } else {
            GL_RENDER_WARNING_WITH_LOCATION(
                    "Unexpected output file extension. "
                    "Changing to '.png'.");
            output_path.replace_extension(".png");
            impl::save_ldr_image(output_path, pixels, resolution, channel);
        }
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

}