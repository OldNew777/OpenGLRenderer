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
        [[nodiscard]] auto width() const noexcept { return _width; }
        [[nodiscard]] auto height() const noexcept { return _height; }
        [[nodiscard]] auto channels() const noexcept { return pixel_storage_channel_count(_pixel_storage); }
        [[nodiscard]] auto size() const noexcept { return _width * _height * pixel_storage_size(_pixel_storage); }
        [[nodiscard]] auto id() const noexcept { return _id; }
        [[nodiscard]] auto handle() const noexcept { return _handle; }

        Texture(const path &image_path) noexcept {
            auto w = 0;
            auto h = 0;

            GL_RENDER_INFO("Loading texture: {}", image_path.string());

            if (LDR_IMAGE_EXT.contains(image_path.extension().string())) {
                auto d = 0;
                uchar4* data = reinterpret_cast<uchar4 *>(stbi_load(image_path.string().c_str(), &w, &h, &d, 4));
                GL_RENDER_ASSERT(data != nullptr, "Failed to load texture: {}", image_path.string());
                // TODO

                stbi_image_free(data);
            } else if (HDR_IMAGE_EXT.contains(image_path.extension().string())) {
                GL_RENDER_ASSERT(IsEXR(image_path.string().c_str()), "Invalid HDR texture: {}", image_path.string());
                const char *err = nullptr;
                float4 *data = nullptr;
                auto ret = LoadEXR(reinterpret_cast<float **>(&data), &w, &h, image_path.string().c_str(), &err);
                if (ret != TINYEXR_SUCCESS && err != nullptr) {
                    GL_RENDER_ERROR("Failed to load texture: {}, {}", image_path.string(), err);
                    FreeEXRErrorMessage(err);
                }
                // TODO
                free(data);
            } else {
                GL_RENDER_ERROR("Unsupported texture format: {}", image_path.extension().string());
            }
            _width = w;
            _height = h;
        }

    protected:
        GLuint _id;
        GLuint _handle;
        uint _width;
        uint _height;
        PixelStorage _pixel_storage;
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