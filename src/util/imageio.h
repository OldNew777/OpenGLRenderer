//
// Created by ChenXin on 2022/9/29.
//

#pragma once

#include <core/stl.h>
#include <core/basic_traits.h>

namespace gl_render {

    [[nodiscard]] bool is_hdr_image(const path& image_path) noexcept;
    [[nodiscard]] bool is_ldr_image(const path& image_path) noexcept;

    struct HDRConfig {
        float exposure = 1.f;
        float gamma = 2.2f;
    };

    void save_image(path output_path, const float *pixels, uint2 resolution, int channel, HDRConfig hdr_config = HDRConfig{}) noexcept;
    void save_image(path output_path, const uchar *pixels, uint2 resolution, int channel) noexcept;
    [[nodiscard]] gl_render::vector<uchar4> load_ldr_image(const path& input_path, uint2& resolution) noexcept;
    [[nodiscard]] gl_render::vector<float4> load_hdr_image(const path& input_path, uint2& resolution) noexcept;

    void hdr2srgb(const float* hdr, uchar* srgb, uint2 resolution, int channel, HDRConfig hdr_config) noexcept;

    template<typename T>
    requires is_vector_v<T>
    void flip_vertically(T* pixels, uint2 resolution) noexcept {
        auto row_size = resolution.x * sizeof(T);
        auto temp_row = vector<T>(row_size);
        for (int i = 0; i < resolution.y / 2; ++i) {
            auto row1 = pixels + i * resolution.x;
            auto row2 = pixels + (resolution.y - i - 1) * resolution.x;
            memcpy(temp_row.data(), row1, row_size);
            memcpy(row1, row2, row_size);
            memcpy(row2, temp_row.data(), row_size);
        }
    }
}