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
#include <base/hdr2ldr.h>
#include <util/imageio.h>
#include <base/light_manager.h>

namespace gl_render {

    class Pipeline {

    public:
        struct Config {
            RendererInfo renderer_info;
            HDRConfig hdr_config;
        };

        static Pipeline& GetInstance(const path &scene_path) noexcept {
            static Pipeline pipeline{scene_path};
            return pipeline;
        }
        void render() noexcept;

    public:
        Pipeline &operator=(Pipeline &&) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

    private:
        explicit Pipeline(const path &scene_path) noexcept;

    private:
        GLFWwindow *_window;
        Config _config;

        gl_render::unique_ptr<SceneAllNode> _scene;
        gl_render::unique_ptr<Geometry> _geometry;
        gl_render::unique_ptr<HDR2LDR> _hdr2ldr;
        gl_render::unique_ptr<LightManager> _lightManager;

        GLuint _hdr_frame_buffer{0u};
    };

}