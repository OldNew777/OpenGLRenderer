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
#include <list>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/constant.h>

namespace gl_render {

    using string = std::basic_string<char, std::char_traits<char>>;
    using std::string_view;

    // stl
    using std::span;
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
}
