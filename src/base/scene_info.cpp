//
// Created by ChenXin on 2022/10/22.
//

#include <base/scene_info.h>

namespace gl_render {

    unordered_map <string, MaterialInfo::MaterialType> MaterialInfo::TYPE_MAP = {
            {
                    "phong", MaterialType::Phong,
            },
    };


}