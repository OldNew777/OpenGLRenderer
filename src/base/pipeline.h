//
// Created by ChenXin on 2022/9/20.
//

#pragma once

#include <filesystem>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <core/logger.h>
#include <core/stl.h>

namespace gl_render {

    class Pipeline {

    public:
        Pipeline(const std::filesystem::path &scene_path) noexcept;

        void render() noexcept;

    private:
        gl_render::string _scene;
        GLFWwindow *_window;
    };

}