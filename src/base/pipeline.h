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
#include <util/imageio.h>

namespace gl_render {

    class Pipeline {

    public:
        static const uint SHADOW_WIDTH, SHADOW_HEIGHT;

        struct Config {
            RendererInfo renderer_info;
            HDRConfig hdr_config;
        };

        static Pipeline& GetInstance(const path &scene_path) noexcept {
            static Pipeline pipeline{scene_path};
            return pipeline;
        }
        void renderShadowMap() noexcept;
        void InitHDRFrameBuffer() noexcept;
        void render() noexcept;

    private:
        Pipeline(const path &scene_path) noexcept;
        Pipeline &operator=(Pipeline &&) = default;
        Pipeline &operator=(const Pipeline &) = delete;

        void renderQuad() noexcept;

    private:
        GLFWwindow *_window;
        Config _config;

        unique_ptr<SceneAllNode> _scene;
        unique_ptr<Geometry> _geometry;
        uint _depth_map_buffer{0u};
        uint _depth_map_tex_buffer{0u};
        uint _hdr_frame_buffer{0u};
        uint _hdr_tex_buffer{0u};
    };

}