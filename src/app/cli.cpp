//
// Created by Kasumi on 2022/7/21.
//

#include <string_view>
#include <iostream>
#include <filesystem>

#include <cxxopts.hpp>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <core/logger.h>
#include <core/stl.h>

#include <base/camera.h>

using namespace gl_render;

[[nodiscard]] auto parse_cli_options(int argc, const char *const *argv) noexcept {
    cxxopts::Options cli{"opengl-render-cli"};
    cli.add_option("", "d", "device", "Compute device index",
                   cxxopts::value<uint32_t>()->default_value("0"), "<index>");
    cli.add_option("", "s", "scene", "Path to scene description file",
                   cxxopts::value<std::filesystem::path>(), "<file>");
    cli.add_option("", "D", "define", "Parameter definitions to override scene description macros.",
                   cxxopts::value<std::vector<std::string>>()->default_value("<none>"), "<key>=<value>");
    cli.add_option("", "h", "help", "Display this help message",
                   cxxopts::value<bool>()->default_value("false"), "");
    cli.allow_unrecognised_options();
    cli.positional_help("<file>");
    cli.parse_positional("scene");
    auto options = [&] {
        try {
            return cli.parse(argc, argv);
        } catch (const std::exception &e) {
            GL_RENDER_WARNING_WITH_LOCATION(
                    "Failed to parse command line arguments: {}.",
                    e.what());
            std::cout << cli.help() << std::endl;
            exit(-1);
        }
    }();
    if (options["help"].as<bool>()) {
        std::cout << cli.help() << std::endl;
        exit(0);
    }
    if (options["scene"].count() == 0u) [[unlikely]] {
        GL_RENDER_WARNING_WITH_LOCATION("Scene file not specified.");
        std::cout << cli.help() << std::endl;
        exit(-1);
    }
    if (auto unknown = options.unmatched(); !unknown.empty()) [[unlikely]] {
        gl_render::string opts{unknown.front()};
        for (auto &&u : gl_render::span{unknown}.subspan(1)) {
            opts.append("; ").append(u);
        }
        GL_RENDER_WARNING_WITH_LOCATION(
                "Unrecognized options: {}", opts);
    }
    return options;
}

int main(int argc, char *argv[]) {
    log_level_info();
    auto options = parse_cli_options(argc, argv);

    auto index = options["device"].as<uint32_t>();
    auto path = options["scene"].as<std::filesystem::path>();
    auto definitions = options["define"].as<std::vector<std::string>>();

    // load scene
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
    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL-Render", nullptr, nullptr);
    if (window == nullptr) {
        GL_RENDER_ERROR_WITH_LOCATION("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // =======================================
    if (gladLoadGL() == 0) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);

    gl_render::queue<double> frame_time;
    double last_fps_time = glfwGetTime();
    double fps_time_sum = 0.f;
    int fps_count = 60;

    while (!glfwWindowShouldClose(window)) {
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
        auto projection = perspective(radians(fov), static_cast<float>(width) / static_cast<float>(height), near_plane, far_plane);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(static_cast<uint32_t>(GL_COLOR_BUFFER_BIT) | static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT));
        glViewport(0, 0, width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}