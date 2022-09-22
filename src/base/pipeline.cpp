//
// Created by ChenXin on 2022/9/20.
//

#include <base/pipeline.h>

#include <fstream>

#include <nlohmann/json.hpp>
//#include <imgui/imgui.h>
//#include <imgui/backends/imgui_impl_glfw.h>
//#include <imgui/backends/imgui_impl_opengl3.h>

#include <core/logger.h>
#include <base/shader.h>
#include <base/camera.h>

namespace gl_render {

    Pipeline::Pipeline(const path &scene_path) noexcept {
        // load scene
        nlohmann::json scene_json = nlohmann::json::parse(std::ifstream{scene_path});
        _scene.emplace(SceneAllNode{scene_json});
        const auto& camera_info = _scene->scene_all_info().camera->camera_info();
        int width = static_cast<int>(camera_info.resolution.x);
        int height = static_cast<int>(camera_info.resolution.y);

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
        glfwSwapInterval(_config.enable_vsync);    // handle vsync

        // glad: load all OpenGL function pointers
        // =======================================
        if (gladLoadGL() == 0) {
            GL_RENDER_ERROR_WITH_LOCATION("Failed to initialize GLAD");
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        if (!_scene->scene_all_info().renderer->renderer_info().enable_two_sided_shading) {
            glCullFace(GL_BACK);
        }
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_MULTISAMPLE);

//        // imgui init
//        IMGUI_CHECKVERSION();
//        ImGui::CreateContext();
//
//        // setup prefered style
//        ImGui::StyleColorsDark();
//
//        // Decide GL+GLSL versions
//        const char *glsl_version = "#version 410 core";
//        // setup platform/renderer bindings
//        ImGui_ImplGlfw_InitForOpenGL(_window, true);
//        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void Pipeline::render() noexcept {
        gl_render::queue<double> frame_time;
        double last_fps_time = glfwGetTime();
        double fps_time_sum = 0.f;
        int fps_count = 60;
        auto clear_color = float3(0.45f, 0.55f, 0.60f);

        const auto& lights = _scene->scene_all_info().lights;
        const auto& camera_info = _scene->scene_all_info().camera->camera_info();
        int width = static_cast<int>(camera_info.resolution.x);
        int height = static_cast<int>(camera_info.resolution.y);
        float near_plane = 0.01f;
        float far_plane = 100.f;

        // build and compile shaders
        Shader shader{
            "data/shaders/ggx.vs",
            "data/shaders/ggx_approx.fs",
            {},
            {
                {std::string{"LIGHT_COUNT"}, serialize(lights.size())},
                {std::string{"TEXTURE_MAX_SIZE"}, serialize(4096)}
            }
        };

        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();

//            // Start the Dear ImGui frame
//            ImGui_ImplOpenGL3_NewFrame();
//            ImGui_ImplGlfw_NewFrame();
//            ImGui::NewFrame();
//
//            ImGui::Begin("Hello, world!");
//            ImGui::Text("Settings");
//            ImGui::Checkbox("Enable shadow", &_config.enable_shadow);


            // Rendering
            glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
            glClear(static_cast<uint32_t>(GL_COLOR_BUFFER_BIT) | static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT));
            auto projection = perspective(
                    radians(camera_info.fov),
                    static_cast<float>(width) / static_cast<float>(height),
                    near_plane, far_plane);
            glViewport(0, 0, width, height);
//            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            shader.use();
            // lights
            for (auto i = 0ul; i < lights.size(); i++) {
                shader.setVec3(serialize("lights[", i, "].Position"), lights[i].light_info().position);
                shader.setVec3(serialize("lights[", i, "].Color"), lights[i].light_info().emission);
            }
            // camera
            Camera camera{camera_info.position, camera_info.front, camera_info.up, camera_info.fov};
            shader.setMat4("projection", projection);
            shader.setMat4("view", camera.view_matrix());
            shader.setVec3("cameraPos", camera_info.position);


            // calculate fps
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

            glfwSwapBuffers(_window);
        }

        // Cleanup
//        ImGui_ImplOpenGL3_Shutdown();
//        ImGui_ImplGlfw_Shutdown();
//        ImGui::DestroyContext();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

}
