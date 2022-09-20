//
// Created by ChenXin on 2022/9/19.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    namespace constant {
        /// pi
        constexpr auto PI = 3.14159265358979323846264338327950288f;
        /// pi/2
        constexpr auto PI_OVER_TWO = 1.57079632679489661923132169163975144f;
        /// pi/4
        constexpr auto PI_OVER_FOUR = 0.785398163397448309615660845819875721f;
        /// 1/pi
        constexpr auto INV_PI = 0.318309886183790671537767526745028724f;
        /// 2/pi
        constexpr auto TWO_OVER_PI = 0.636619772367581343075535053490057448f;
        /// sqrt(2)
        constexpr auto SQRT_TWO = 1.41421356237309504880168872420969808f;
        /// 1/sqrt(2)
        constexpr auto INV_SQRT_TWO = 0.707106781186547524400844362104849039f;
        /// epsilon
        constexpr auto EPSILON = 0x1p-24f;
        /// 1-epsilon
        constexpr auto ONE_MINUS_EPSILON = 0x1.fffffep-1f;  // = 1 - EPSILON

        /// float4x4 Identity
        constexpr auto IDENTITY_FLOAT4x4 = float4x4{1.f, 0.f, 0.f, 0.f,
                                                   0.f, 1.f, 0.f, 0.f,
                                                   0.f, 0.f, 1.f, 0.f,
                                                   0.f, 0.f, 0.f, 1.f};
    }

}// namespace luisa::constants