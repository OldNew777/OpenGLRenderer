//
// Created by ChenXin on 2022/10/23.
//

#pragma once

#include <stb/stb_image.h>
#include <tinyexr.h>
#include <glad/glad.h>

#include <core/stl.h>
#include <base/pixel.h>
#include <core/logger.h>
#include <util/imageio.h>

namespace gl_render {

    class Texture {
    public:
        [[nodiscard]] auto resolution() const noexcept { return _resolution; }
        [[nodiscard]] auto channels() const noexcept { return pixel_storage_channel_count(_pixel_storage); }
        [[nodiscard]] auto size() const noexcept { return _resolution.x * _resolution.y * pixel_storage_size(_pixel_storage); }
        [[nodiscard]] auto id() const noexcept { return _id; }
        [[nodiscard]] auto handle() const noexcept { return _handle; }

        explicit Texture(const path &image_path) noexcept {
            GL_RENDER_INFO("Loading texture: {}", image_path.string());

            glGenTextures(1, &_id);
            glBindTexture(GL_TEXTURE_2D, _id);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (is_ldr_image(image_path)) {
                auto image = load_ldr_image(image_path, _resolution);
                _pixel_storage = PixelStorage::BYTE4;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _resolution.x, _resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
            } else if (is_hdr_image(image_path)) {
                auto image = load_hdr_image(image_path, _resolution);
                _pixel_storage = PixelStorage::FLOAT4;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _resolution.x, _resolution.y, 0, GL_RGBA, GL_FLOAT, image.data());
            } else {
                GL_RENDER_ERROR("Unsupported texture format: {}", image_path.extension().string());
            }
            glGenerateMipmap(GL_TEXTURE_2D);
            _handle = glGetTextureHandleARB(_id);
            glMakeTextureHandleResidentARB(_handle);
            glBindTexture(GL_TEXTURE_2D, 0);
            GL_RENDER_INFO("Created texture: {}, id: {}, handle: {}", image_path.string(), _id, _handle);
        }

        ~Texture() {
            glDeleteTextures(1, &_id);
        }

    protected:
        GLuint _id = 0u;
        GLuint64 _handle;
        uint2 _resolution;
        PixelStorage _pixel_storage = PixelStorage::BYTE4;
    };

}