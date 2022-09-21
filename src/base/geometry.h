//
// Created by ChenXin on 2022/9/21.
//

#pragma once

#include <base/scene_parser.h>

namespace gl_render {

    class Geometry {
    public:
        Geometry(vector<MeshNode> meshes, unordered_map<string, MaterialNode> materials) noexcept;
    };

}