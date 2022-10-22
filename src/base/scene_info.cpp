//
// Created by ChenXin on 2022/10/22.
//

#include <base/scene_info.h>

namespace gl_render {

    unordered_map <string, MaterialInfo::MaterialType> MaterialInfo::STRING2TYPE = {
            {
                    "phong", MaterialType::Phong,
            },
    };

    unordered_map <MaterialInfo::MaterialType, string> MaterialInfo::TYPE2STRING = {
            {
                    MaterialType::Phong, "phong",
            },
    };

}