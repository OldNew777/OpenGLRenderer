//
// Created by ChenXin on 2022/9/20.
//

#pragma once

#include <filesystem>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <core/logger.h>
#include <core/stl.h>
#include <base/scene_info.h>

namespace gl_render {

    class Pipeline {

    public:

        struct Config {
            bool enable_vsync = false;
            bool enable_shadow = false;
        };

        static Pipeline& GetInstance(const std::filesystem::path &scene_path) noexcept {
            static Pipeline pipeline{scene_path};
            return pipeline;
        }
        void render() noexcept;

    private:
        Pipeline(const std::filesystem::path &scene_path) noexcept;

    private:
        optional<SceneAllNode> _scene;
        GLFWwindow *_window;
        Config _config;
    };

}