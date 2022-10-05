//
// Created by ChenXin on 2022/9/28.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    namespace util {

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

        [[nodiscard]] inline float distance(float3 point, float4 plane) noexcept {
            return abs(point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.w)
                   / sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        }

        [[nodiscard]] float get_far_plane(float3 point, float3 dir, impl::AABB aabb) noexcept {
            auto far_plane = 0.f;
            auto plane = float4{dir.x, dir.y, dir.z, 0.f};
            for (auto i = 0; i < 2; ++i) {
                for (auto j = 0; j < 2; ++j) {
                    for (auto k = 0; k < 2; ++k) {
                        plane.w = -(aabb[i].x * dir.x + aabb[j].y * dir.y + aabb[k].z * dir.z);
                        far_plane = max(far_plane, distance(point, plane));
                    }
                }
            }
            return far_plane;
        }

    }
}