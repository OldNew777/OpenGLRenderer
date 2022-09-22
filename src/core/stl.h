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
    using std::span;
    using std::optional;
    using std::nullopt;
    using std::unique_ptr;
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
