//
// Created by ChenXin on 2022/10/25.
//

#pragma once

#include <base/texture.h>

namespace gl_render {

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