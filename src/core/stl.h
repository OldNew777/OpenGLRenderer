//
// Created by Kasumi on 2022/9/13.
//

#pragma once

#include <cstdlib>
#include <cmath>
#include <memory>
#include <string>
#include <span>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>

namespace gl_render {

    using string = std::basic_string<char, std::char_traits<char>>;
    using std::string_view;

    using std::span;
    using std::vector;
    using std::map;
    using std::unordered_map;
    using std::unordered_set;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using float2x2 = glm::mat2x2;
    using float3x3 = glm::mat3x3;
    using float4x4 = glm::mat4x4;

    using glm::normalize;
    using glm::cross;
    using glm::dot;
    using glm::length;
    using glm::distance;

    using glm::inverse;
    using glm::transpose;
}
