//
// Created by Kasumi on 2022/7/21.
//

#include <string_view>
#include <iostream>

#include <cxxopts.hpp>

#include <base/camera.h>
#include <base/pipeline.h>
#include <core/logger.h>

using namespace gl_render;

[[nodiscard]] auto parse_cli_options(int argc, const char *const *argv) noexcept {
    cxxopts::Options cli{"opengl-render-cli"};
    cli.add_option("", "d", "device", "Compute device index",
                   cxxopts::value<uint32_t>()->default_value("0"), "<index>");
    cli.add_option("", "s", "scene", "Path to scene description file",
                   cxxopts::value<path>(), "<file>");
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

    auto device_index = options["device"].as<uint32_t>();
    auto scene_path = options["scene"].as<path>();

    auto pipeline = Pipeline::GetInstance(scene_path);
    pipeline.render();

    return 0;
}