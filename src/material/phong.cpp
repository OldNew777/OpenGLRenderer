//
// Created by ChenXin on 2022/10/22.
//

#include <base/scene_info.h>

namespace gl_render {

    class PhongMaterialInfo : public MaterialInfo {
    public:
        PhongMaterialInfo() noexcept {
            type = MaterialType::Phong;
        }
    };

}