//
// Created by ChenXin on 2022/9/29.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    void save_image(path output_path, const float *pixels, uint2 resolution, int channel = 3) noexcept;

    void save_image(path output_path, const uchar *pixels, uint2 resolution, int channel = 3) noexcept;

}