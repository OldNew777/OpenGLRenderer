//
// Created by ChenXin on 2022/9/29.
//

#include "imageio.h"

#include <tinyexr.h>
#include <stb/stb_image_write.h>

#include <core/logger.h>

namespace gl_render {

    void save_image(path output_path, const float *pixels, uint2 resolution, int channel) noexcept {
        // save results
        GL_RENDER_ASSERT(channel == 3 || channel == 4, "Image saving only supports 3 or 4 channels");
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
            int ret = stbi_write_hdr(output_path.string().c_str(), size.x, size.y, channel, reinterpret_cast<const float *>(pixels));
            if (ret == 0) [[unlikely]] {
                GL_RENDER_WARNING_WITH_LOCATION(
                        "Failed to save image to '{}'.",
                        output_path.string());
            }
        }
    }

    void save_image(path output_path, const uchar *pixels, uint2 resolution, int channel) noexcept {
        // save results
        GL_RENDER_ASSERT(channel == 3 || channel == 4, "Image saving only supports 3 or 4 channels");
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
    }

}