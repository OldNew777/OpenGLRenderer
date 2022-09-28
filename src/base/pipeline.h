//
// Created by ChenXin on 2022/9/20.
//

#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <core/logger.h>
#include <core/stl.h>
#include <base/scene_parser.h>
#include <base/geometry.h>

namespace gl_render {

    class Pipeline {

    public:

        struct Config {
            bool enable_vsync = false;
            bool enable_shadow = false;
        };

        static Pipeline& GetInstance(const path &scene_path) noexcept {
            static Pipeline pipeline{scene_path};
            return pipeline;
        }
        void render() noexcept;

    private:
        Pipeline(const path &scene_path) noexcept;
        Pipeline &operator=(Pipeline &&) = default;
        Pipeline &operator=(const Pipeline &) = delete;

    private:
        unique_ptr<SceneAllNode> _scene;
        unique_ptr<Geometry> _geometry;
        GLFWwindow *_window;
        Config _config;
    };

}