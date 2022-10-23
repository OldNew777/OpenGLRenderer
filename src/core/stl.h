//
// Created by Kasumi on 2022/9/13.
//

#pragma once

#include <cstdlib>
#include <sstream>
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
#include <filesystem>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>

#include <core/basic_traits.h>

namespace gl_render {

    using string = std::basic_string<char, std::char_traits<char>>;
    using std::string_view;
    using std::stringstream;
    using std::filesystem::path;

    // stl
    using std::tuple;
    using std::pair;
    using std::function;
    using std::span;
    using std::optional;
    using std::nullopt;
    using std::unique_ptr;
    using std::make_unique;
    using std::vector;
    using std::list;
    using std::queue;
    using std::map;
    using std::unordered_map;
    using std::unordered_set;

    // data type
    using float2 = Vector<glm::f32, 2>;
    using float3 = Vector<glm::f32, 3>;
    using float4 = Vector<glm::f32, 4>;
    using float2x2 = Matrix<2>;
    using float3x3 = Matrix<3>;
    using float4x4 = Matrix<4>;
    using uint = glm::u32;
    using uint2 = Vector<glm::u32, 2>;
    using uint3 = Vector<glm::u32, 3>;
    using uint4 = Vector<glm::u32, 4>;
    using int2 = Vector<glm::i32, 2>;
    using int3 = Vector<glm::i32, 3>;
    using int4 = Vector<glm::i32, 4>;
    using uchar = glm::u8;
    using uchar2 = Vector<glm::u8, 2>;
    using uchar3 = Vector<glm::u8, 3>;
    using uchar4 = Vector<glm::u8, 4>;

    // glm
    using glm::normalize;
    using glm::cross;
    using glm::dot;
    using glm::length;
    using glm::distance;

    using glm::inverse;
    using glm::transpose;
    using glm::scale;
    using glm::rotate;
    using glm::translate;

    using glm::perspective;
    using glm::ortho;
    using glm::lookAt;

    // function
    using glm::radians;
    using glm::degrees;
    using glm::max;
    using glm::min;
    using glm::clamp;
    using glm::sqrt;
    using glm::pow;

    template<typename FMT, typename... Args>
    [[nodiscard]] inline auto format(FMT &&f, Args &&...args) noexcept {
        using memory_buffer = fmt::basic_memory_buffer<char, fmt::inline_buffer_size>;
        memory_buffer buffer;
        fmt::format_to(std::back_inserter(buffer), std::forward<FMT>(f), std::forward<Args>(args)...);
        return gl_render::string{buffer.data(), buffer.size()};
    }

    template<typename T>
    inline T lerp(T u, T v, float t) {
        return (1 - t) * u + t * v;
    }

    template<typename T>
    requires is_vector_v<T>
    [[nodiscard]] constexpr inline auto to_string(T v) noexcept {
        constexpr auto length = vector_dimension_v<T>;
        if constexpr (length == 1) {
            return format("{}", v.x);
        } else if constexpr (length == 2) {
            return format("({}, {})", v.x, v.y);
        } else if constexpr (length == 3) {
            return format("({}, {}, {})", v.x, v.y, v.z);
        } else if constexpr (length == 4) {
            return format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
        } else {
            return "Unknown vector type";
        }
    }
}
