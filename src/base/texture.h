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

            if (is_ldr_image(image_path)) {
                auto image = load_ldr_image(image_path, _resolution);
                _pixel_storage = PixelStorage::BYTE4;
                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _resolution.x, _resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
                glGenerateMipmap(GL_TEXTURE_2D);
                _handle = glGetTextureHandleARB(_id);
                glMakeTextureHandleResidentARB(_handle);
                glBindTexture(GL_TEXTURE_2D, 0);
            } else if (is_hdr_image(image_path)) {
                auto image = load_hdr_image(image_path, _resolution);
                _pixel_storage = PixelStorage::FLOAT4;
                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _resolution.x, _resolution.y, 0, GL_RGBA, GL_FLOAT, image.data());
                glGenerateMipmap(GL_TEXTURE_2D);
                _handle = glGetTextureHandleARB(_id);
                glMakeTextureHandleResidentARB(_handle);
                glBindTexture(GL_TEXTURE_2D, 0);
            } else {
                GL_RENDER_ERROR("Unsupported texture format: {}", image_path.extension().string());
            }
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

    class TextureManager {
    private:
        TextureManager() noexcept = default;

    public:
        static TextureManager* GetInstance() noexcept {
            static TextureManager instance;
            return &instance;
        }

        Texture* CreateTexture(const path &image_path) noexcept {
            if (auto iter = _textures.find(image_path); iter == _textures.end()) {
                _textures.emplace(image_path, gl_render::make_unique<Texture>(image_path));
            } else {
                GL_RENDER_INFO("Using cached image: {}", image_path.string());
            }
            return _textures[image_path].get();
        }
        [[nodiscard]] Texture* GetTexture(const path &image_path) const noexcept {
            return _textures.at(image_path).get();
        }

    private:
        gl_render::unordered_map<path, gl_render::unique_ptr<Texture>> _textures;

    };

}