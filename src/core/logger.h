//
// Created by Kasumi on 2022/9/13.
//

#pragma once

#include <iostream>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <core/stl.h>
#include <core/macro.h>

namespace gl_render {

    namespace detail {
        [[nodiscard]] spdlog::logger &default_logger() noexcept;
    }

    template<typename... Args>
    inline void log_trace(Args &&...args) noexcept {
        detail::default_logger().trace(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void log_debug(Args &&...args) noexcept {
        detail::default_logger().debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void log_info(Args &&...args) noexcept {
        detail::default_logger().info(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void log_warning(Args &&...args) noexcept {
        detail::default_logger().warn(std::forward<Args>(args)...);
    }

    template<typename... Args>
    [[noreturn]] GL_RENDER_FORCE_INLINE void log_error(Args &&...args) noexcept {
        detail::default_logger().error(std::forward<Args>(args)...);
        std::abort();
    }

    /// Set log level as trace
    void log_level_trace() noexcept;

    /// Set log level as debug
    void log_level_debug() noexcept;

    /// Set log level as info
    void log_level_info() noexcept;

    /// Set log level as warning
    void log_level_warning() noexcept;

    /// Set log level as error
    void log_level_error() noexcept;

}

/**
 * @brief Verbose logging
 * 
 * Ex. GL_RENDER_TRACE("function {} returns {}", functionName, functionReturnInt);
 */
#define GL_RENDER_TRACE(fmt, ...) \
    ::gl_render::log_trace(FMT_STRING(fmt) __VA_OPT__(, ) __VA_ARGS__)
/**
 * @brief Debug logging
 *
 * Ex. GL_RENDER_DEBUG("function {} returns {}", functionName, functionReturnInt);
 */
#define GL_RENDER_DEBUG(fmt, ...) \
    ::gl_render::log_debug(FMT_STRING(fmt) __VA_OPT__(, ) __VA_ARGS__)
/**
 * @brief Info logging
 * 
 * Ex. GL_RENDER_INFO("function {} returns {}", functionName, functionReturnInt);
 */
#define GL_RENDER_INFO(fmt, ...) \
    ::gl_render::log_info(FMT_STRING(fmt) __VA_OPT__(, ) __VA_ARGS__)
/**
 * @brief Warning logging
 * 
 * Ex. GL_RENDER_WARNING("function {} returns {}", functionName, functionReturnInt);
 */
#define GL_RENDER_WARNING(fmt, ...) \
    ::gl_render::log_warning(FMT_STRING(fmt) __VA_OPT__(, ) __VA_ARGS__)
/**
 * @brief Error logging
 * 
 * After logging error message, the program will be aborted.
 * Ex. GL_RENDER_ERROR("function {} returns {}", functionName, functionReturnInt);
 */
#define GL_RENDER_ERROR(fmt, ...) \
    ::gl_render::log_error(FMT_STRING(fmt) __VA_OPT__(, ) __VA_ARGS__)

/// GL_RENDER_TRACE with file and line information
#define GL_RENDER_TRACE_WITH_LOCATION(fmt, ...) \
    GL_RENDER_TRACE(fmt " [{}:{}]" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)
/// GL_RENDER_DEBUG with file and line information
#define GL_RENDER_DEBUG_WITH_LOCATION(fmt, ...) \
    GL_RENDER_DEBUG(fmt " [{}:{}]" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)
/// GL_RENDER_INFO with file and line information
#define GL_RENDER_INFO_WITH_LOCATION(fmt, ...) \
    GL_RENDER_INFO(fmt " [{}:{}]" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)
/// GL_RENDER_WARNING with file and line information
#define GL_RENDER_WARNING_WITH_LOCATION(fmt, ...) \
    GL_RENDER_WARNING(fmt " [{}:{}]" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)
/// GL_RENDER_ERROR with file and line information
#define GL_RENDER_ERROR_WITH_LOCATION(fmt, ...) \
    GL_RENDER_ERROR(fmt " [{}:{}]" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)

#define GL_RENDER_ASSERT(x, fmt, ...)            \
    do {                                         \
        if (!(x)) [[unlikely]] {                 \
            auto msg = gl_render::format(        \
                fmt __VA_OPT__(, ) __VA_ARGS__); \
            GL_RENDER_ERROR_WITH_LOCATION(       \
                "Assertion \"{}\" failed: {}",   \
                #x, msg);                        \
        }                                        \
    } while (false)