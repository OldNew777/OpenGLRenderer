//
// Created by ChenXin on 2022/9/20.
//

#include <base/pipeline.h>

#include <fstream>

#include <nlohmann/json.hpp>
#include <stb/stb_image_write.h>
//#include <imgui/imgui.h>
//#include <imgui/backends/imgui_impl_glfw.h>
//#include <imgui/backends/imgui_impl_opengl3.h>

#include <core/logger.h>
#include <base/shader.h>
#include <base/camera.h>
#include <util/imageio.h>
#include <core/util.h>

namespace gl_render {

    Pipeline::Pipeline(const path &scene_path) noexcept {
        // load scene
        nlohmann::json scene_json = nlohmann::json::parse(std::ifstream{scene_path});
        _scene = make_unique<SceneAllNode>(scene_json);
        const auto& camera_info = _scene->scene_all_info().camera->camera_info();
        _config.renderer_info = _scene->scene_all_info().renderer->renderer_info();
        if (_config.renderer_info.output_file.is_relative()) {
            _config.renderer_info.output_file = scene_path.parent_path() / _config.renderer_info.output_file;
        }
        int width = static_cast<int>(camera_info.resolution.x);
        int height = static_cast<int>(camera_info.resolution.y);

        // glfw init
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//        glfwWindowHint(GLFW_SRGB_CAPABLE, 1);
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
        if (!_config.renderer_info.enable_two_sided_shading) {
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

        _geometry = make_unique<Geometry>(_scene->scene_all_info(), scene_path.parent_path());

        stbi_flip_vertically_on_write(true);
    }

    void Pipeline::render() noexcept {
        gl_render::queue<double> frame_time;
        double last_fps_time = glfwGetTime();
        double fps_time_sum = 0.f;
        const auto fps_count = 600u;
        auto print_count = 0u;
        auto clear_color = float3(0.45f, 0.55f, 0.60f);

        const auto& lights = _scene->scene_all_info().lights;
        const auto& camera_info = _scene->scene_all_info().camera->camera_info();
        int width = static_cast<int>(camera_info.resolution.x);
        int height = static_cast<int>(camera_info.resolution.y);
        float near_plane = 0.01f;
        float far_plane = get_far_plane(camera_info.position, camera_info.front, _geometry->aabb()) * 1.1f;
        Camera camera{camera_info.position, camera_info.front, camera_info.up, camera_info.fov};
        auto view_matrix = camera.view_matrix();
        auto projection = perspective(
                radians(camera_info.fov),
                static_cast<float>(width) / static_cast<float>(height),
                near_plane, far_plane);

        // build and compile shaders
        Shader shader{
            "data/shaders/matte.vs",
            "data/shaders/matte.fs",
            {},
            {
                {std::string{"POINT_LIGHT_COUNT"}, serialize(lights.size())},
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
            glViewport(0, 0, width, height);
//            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            shader.use();
            // lights
            // point lights
            for (auto i = 0ul; i < lights.size(); i++) {
                shader.setVec3(serialize("pointLights[", i, "].Position"), lights[i].light_info().position);
                shader.setVec3(serialize("pointLights[", i, "].Color"), lights[i].light_info().emission);
            }
            // camera
            shader.setMat4("projection", projection);
            shader.setMat4("view", view_matrix);
            shader.setVec3("cameraPos", camera_info.position);
            _geometry->render(shader);

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

            if (print_count++ == fps_count) {
                print_count = 0;
                GL_RENDER_INFO(
                        "FPS: {}, SPF: {}",
                        1.0 / fps_time_sum * frame_time.size(),
                        fps_time_sum / frame_time.size());
            }

            glfwSwapBuffers(_window);
        }

        // save to file
        vector<uchar3> pixels(width * height);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        glReadBuffer(GL_BACK);
        save_image(_config.renderer_info.output_file, (const uchar*)pixels.data(), uint2{width, height}, 3);

//        // Cleanup
//        ImGui_ImplOpenGL3_Shutdown();
//        ImGui_ImplGlfw_Shutdown();
//        ImGui::DestroyContext();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

}
