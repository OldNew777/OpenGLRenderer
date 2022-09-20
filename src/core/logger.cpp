//
// Created by Kasumi on 2022/9/13.
//

#include <core/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace gl_render {

    namespace detail {
        [[nodiscard]] spdlog::logger &default_logger() noexcept {
            static auto logger = [] {
                auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                spdlog::logger l{"console", sink};
                l.flush_on(spdlog::level::err);
#ifndef NDEBUG
                l.set_level(spdlog::level::debug);
#else
                l.set_level(spdlog::level::info);
#endif
                return l;
            }();
            return logger;
        }
    }

    void log_level_trace() noexcept { detail::default_logger().set_level(spdlog::level::trace); }
    void log_level_debug() noexcept { detail::default_logger().set_level(spdlog::level::debug); }
    void log_level_info() noexcept { detail::default_logger().set_level(spdlog::level::info); }
    void log_level_warning() noexcept { detail::default_logger().set_level(spdlog::level::warn); }
    void log_level_error() noexcept { detail::default_logger().set_level(spdlog::level::err); }

}// namespace gl_render
