//
// Created by Kasumi on 2022/9/13.
//

#pragma once

#include <cstdlib>
#include <cmath>
#include <memory>
#include <string>
#include <span>
#include <optional>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>

namespace gl_render {

    using string = std::basic_string<char, std::char_traits<char>>;
    using std::string_view;

    // stl
    using std::span;
    using std::optional;
    using std::vector;
    using std::list;
    using std::queue;
    using std::map;
    using std::unordered_map;
    using std::unordered_set;

    // data type
    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using float2x2 = glm::mat2x2;
    using float3x3 = glm::mat3x3;
    using float4x4 = glm::mat4x4;
    using uint = unsigned int;
    using uint2 = glm::uvec2;
    using uint3 = glm::uvec3;
    using uint4 = glm::uvec4;

    template<typename T>
    [[nodiscard]] inline constexpr bool is_vector_v() noexcept {
        return false;
    }
#define GL_RENDER_PROCESS_VECTOR(type)                                  \
    template<>                                                          \
    [[nodiscard]] inline constexpr bool is_vector_v<type>() noexcept {  \
        return true;                                                    \
    }
    GL_RENDER_PROCESS_VECTOR(float2)
    GL_RENDER_PROCESS_VECTOR(float3)
    GL_RENDER_PROCESS_VECTOR(float4)
    GL_RENDER_PROCESS_VECTOR(uint2)
    GL_RENDER_PROCESS_VECTOR(uint3)
    GL_RENDER_PROCESS_VECTOR(uint4)
#undef GL_RENDER_PROCESS_VECTOR

    template<typename T>
    [[nodiscard]] inline constexpr bool is_matrix_v() noexcept {
        return false;
    }
#define GL_RENDER_PROCESS_MATRIX(type)                                  \
    template<>                                                          \
    [[nodiscard]] inline constexpr bool is_matrix_v<type>() noexcept {  \
        return true;                                                    \
    }
    GL_RENDER_PROCESS_MATRIX(float2x2)
    GL_RENDER_PROCESS_MATRIX(float3x3)
    GL_RENDER_PROCESS_MATRIX(float4x4)
#undef GL_RENDER_PROCESS_MATRIX

    template<typename T>
    [[nodiscard]] inline constexpr bool is_scalar_v() noexcept {
        return true;
    }
#define GL_RENDER_PROCESS_SCALAR(type)                                  \
    template<>                                                          \
    [[nodiscard]] inline constexpr bool is_scalar_v<type>() noexcept {  \
        return false;                                                   \
    }
    GL_RENDER_PROCESS_SCALAR(float2)
    GL_RENDER_PROCESS_SCALAR(float3)
    GL_RENDER_PROCESS_SCALAR(float4)
    GL_RENDER_PROCESS_SCALAR(uint2)
    GL_RENDER_PROCESS_SCALAR(uint3)
    GL_RENDER_PROCESS_SCALAR(uint4)
    GL_RENDER_PROCESS_SCALAR(float2x2)
    GL_RENDER_PROCESS_SCALAR(float3x3)
    GL_RENDER_PROCESS_SCALAR(float4x4)
#undef GL_RENDER_PROCESS_SCALAR

    // glm
    using glm::normalize;
    using glm::cross;
    using glm::dot;
    using glm::length;
    using glm::distance;

    using glm::inverse;
    using glm::transpose;

    using glm::perspective;
    using glm::lookAt;

    // function
    using glm::radians;
    using glm::degrees;

    template<typename FMT, typename... Args>
    [[nodiscard]] inline auto format(FMT &&f, Args &&...args) noexcept {
        using memory_buffer = fmt::basic_memory_buffer<char, fmt::inline_buffer_size>;
        memory_buffer buffer;
        fmt::format_to(std::back_inserter(buffer), std::forward<FMT>(f), std::forward<Args>(args)...);
        return gl_render::string{buffer.data(), buffer.size()};
    }
}
