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
        const auto &camera_info = _scene->scene_all_info().camera->camera_info();
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
        glfwSwapInterval(_config.renderer_info.enable_vsync);    // handle vsync

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

        // init geometry
        _geometry = make_unique<Geometry>(_scene->scene_all_info(), scene_path.parent_path());
        // init shadow map
        InitShadowMap();
        // init hdr frame buffer
        InitHDRFrameBuffer();
    }

    void Pipeline::render() noexcept {
        gl_render::queue<double> frame_time;
        double last_fps_time = glfwGetTime();
        double fps_time_sum = 0.f;
        const auto fps_count_time = 1.f;
        const auto frame_min_size = 60u;
        auto print_time = 0.f;
        auto frame_index = 0u;
        auto clear_color = float3(0.45f, 0.55f, 0.60f);

        const auto &lights = _scene->scene_all_info().lights;
        const auto &camera_info = _scene->scene_all_info().camera->camera_info();
        int width = static_cast<int>(camera_info.resolution.x);
        int height = static_cast<int>(camera_info.resolution.y);
        float near_plane = 0.001f;
        float far_plane = util::get_far_plane(camera_info.position, camera_info.front, _geometry->aabb()) * 1.1f;
        Camera camera{camera_info.position, camera_info.front, camera_info.up, camera_info.fov};
        auto view_matrix = camera.view_matrix();
        auto projection = perspective(
                radians(camera_info.fov),
                static_cast<float>(width) / static_cast<float>(height),
                near_plane, far_plane);

        // build and compile shaders
        Shader shader{
                "data/shaders/phong.vert",
                "data/shaders/phong.frag",
                {},
                {
                        {std::string{"POINT_LIGHT_COUNT"}, serialize(lights.size())},
                        {std::string{"TEXTURE_MAX_SIZE"}, serialize(4096)}
                }
        };
        Shader hdrShader{
                "data/shaders/hdr2ldr.vert",
                "data/shaders/hdr2ldr.frag",
                {},
                {}
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
            glViewport(0, 0, width, height);
//            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // 1. render into hdr framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, _hdr_frame_buffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
//            _geometry->shadow(shader);

            // 2. now render hdr buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            hdrShader.use();
            hdrShader.setBool("hdr", _config.hdr);
            hdrShader.setFloat("exposure", _config.exposure);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _hdr_tex_buffer);
            renderQuad();

            if (auto error = glGetError(); error != GL_NO_ERROR) {
                GL_RENDER_ERROR_WITH_LOCATION("OpenGL error: ", error);
            }

            // calculate fps
            frame_index++;
            double current_time = glfwGetTime();
            double delta_time = current_time - last_fps_time;
            last_fps_time = current_time;
            fps_time_sum += delta_time;
            frame_time.push(delta_time);
            while (fps_time_sum >= fps_count_time && frame_time.size() > frame_min_size) {
                fps_time_sum -= frame_time.front();
                frame_time.pop();
            }

            if (print_time += delta_time; print_time >= fps_count_time) {
                print_time = 0.f;
                GL_RENDER_INFO(
                        "Frame {}, FPS: {}, SPF: {}",
                        frame_index,
                        1.0 / fps_time_sum * frame_time.size(),
                        fps_time_sum / frame_time.size());
            }

            glfwSwapBuffers(_window);
        }

        // save to file
        glBindFramebuffer(GL_FRAMEBUFFER, _hdr_frame_buffer);
        vector<float3> pixels(width * height);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, pixels.data());
        flip_vertically(pixels.data(), uint2{width, height});
        save_image(_config.renderer_info.output_file, (const float *) pixels.data(), uint2{width, height}, 3);

//        // Cleanup
//        ImGui_ImplOpenGL3_Shutdown();
//        ImGui_ImplGlfw_Shutdown();
//        ImGui::DestroyContext();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Pipeline::InitShadowMap() noexcept {
        // depth map VBO
        // -----------------------
        glGenFramebuffers(1, &_depth_map_buffer);
        // create depth texture
        glGenTextures(1, &_depth_map_tex_buffer);
        glBindTexture(GL_TEXTURE_2D, _depth_map_tex_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        // may reduce shadow border artifacts
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, _depth_map_buffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth_map_tex_buffer, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const uint Pipeline::SHADOW_HEIGHT = 1024u;
    const uint Pipeline::SHADOW_WIDTH = 1024u;

    void Pipeline::InitHDRFrameBuffer() noexcept {
        // configure floating point framebuffer
        auto width = _scene->scene_all_info().camera->camera_info().resolution.x;
        auto height = _scene->scene_all_info().camera->camera_info().resolution.y;
        glGenFramebuffers(1, &_hdr_frame_buffer);
        // create floating point color buffer
        glGenTextures(1, &_hdr_tex_buffer);
        glBindTexture(GL_TEXTURE_2D, _hdr_tex_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // create depth buffer (renderbuffer)
        uint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        // attach buffers
        glBindFramebuffer(GL_FRAMEBUFFER, _hdr_frame_buffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _hdr_tex_buffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            GL_RENDER_ERROR("Framebuffer not complete!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Pipeline::renderQuad() noexcept {
        static uint quadVAO = 0u;
        static uint quadVBO = 0u;
        if (quadVAO == 0u) {
            static float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

}
