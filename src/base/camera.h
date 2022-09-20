//
// Created by ChenXin on 2022/9/19.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    class Camera {
    public:
        // camera Attributes
        float3 _position;
        float3 _front;
        float3 _up;
        float3 _right;
        // TODO: default pinhole camera now
        float _fov;

        // constructor with vectors
        Camera(float3 position = float3{0.0f, 0.0f, 0.0f},
               float3 front = float3{0.0f, 0.0f, -1.0f},
               float3 up = float3{0.0f, 1.0f, 0.0f},
               float fov = 35.f);

        [[nodiscard]] virtual float4x4 getViewMatrix() noexcept;
        [[nodiscard]] virtual float getFov() noexcept;
    };

}