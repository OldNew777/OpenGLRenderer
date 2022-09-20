//
// Created by ChenXin on 2022/9/20.
//

#include <base/pipeline.h>

#include <core/logger.h>

namespace gl_render {

    Pipeline::Pipeline(const std::filesystem::path &scene_path) noexcept {
        // TODO: load scene
        int width = 1280;
        int height = 720;
        float fov = 35.f;
        float near_plane = 0.01f;
        float far_plane = 1.f;

        // glfw init
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SRGB_CAPABLE, 1);
        // glfw window creation
        // ====================
        glfwWindowHint(GLFW_SAMPLES, 4);
        _window = glfwCreateWindow(width, height, "OpenGL-Render", nullptr, nullptr);
        if (_window == nullptr) {
            glfwTerminate();
            GL_RENDER_ERROR_WITH_LOCATION("Failed to create GLFW window");
        }
        glfwMakeContextCurrent(_window);

        // glad: load all OpenGL function pointers
        // =======================================
        if (gladLoadGL() == 0) {
            GL_RENDER_ERROR_WITH_LOCATION("Failed to initialize GLAD");
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_MULTISAMPLE);
    }

    void Pipeline::render() noexcept {
        gl_render::queue<double> frame_time;
        double last_fps_time = glfwGetTime();
        double fps_time_sum = 0.f;
        int fps_count = 60;

        while (!glfwWindowShouldClose(_window)) {
            double current_time = glfwGetTime();
            double delta_time = current_time - last_fps_time;
            last_fps_time = current_time;
            fps_time_sum += delta_time;
            frame_time.push(delta_time);
            if (frame_time.size() == fps_count) {
                fps_time_sum -= frame_time.front();
                frame_time.pop();
            }

            GL_RENDER_INFO("FPS: {}", 1.0 / fps_time_sum * frame_time.size());
            GL_RENDER_INFO("SPF: {}", fps_time_sum / frame_time.size());
//            auto projection = perspective(radians(fov), static_cast<float>(width) / static_cast<float>(height), near_plane, far_plane);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(static_cast<uint32_t>(GL_COLOR_BUFFER_BIT) | static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT));
//            glViewport(0, 0, width, height);

            glfwSwapBuffers(_window);
            glfwPollEvents();
        }

        glfwTerminate();
    }

}
