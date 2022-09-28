//
// Created by ChenXin on 2022/9/28.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    [[nodiscard]] constexpr size_t next_power_of_two(size_t x) {
        if (x == 0) { return 0; }
        x--;
        x |= x >> 1u;
        x |= x >> 2u;
        x |= x >> 4u;
        x |= x >> 8u;
        x |= x >> 16u;
        x |= x >> 32u;
        return ++x;
    }

    [[nodiscard]] constexpr size_t log2_exact(size_t x) noexcept {
        auto index = 0ull;
        for (; x != 1ull; x >>= 1ull) { index++; }
        return index;
    }

    [[nodiscard]] constexpr size_t log2(size_t x) noexcept {
        return log2_exact(next_power_of_two(x));
    }

}