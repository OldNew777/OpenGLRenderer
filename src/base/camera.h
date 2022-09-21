//
// Created by ChenXin on 2022/9/19.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    class Camera {
    private:
        // camera Attributes
        float3 _position;
        float3 _front;
        float3 _up;
        float3 _right;
        // TODO: default pinhole camera now
        float _fov;

    public:
        // constructor with vectors
        Camera(float3 position = float3{0.0f, 0.0f, 0.0f},
               float3 front = float3{0.0f, 0.0f, -1.0f},
               float3 up = float3{0.0f, 1.0f, 0.0f},
               float fov = 35.f)
                : _position(position), _front(front), _up(up), _fov(fov) {
            _front = normalize(_front);
            _right = normalize(cross(_front, _up));
            _up = normalize(cross(_right, _front));
        }

        [[nodiscard]] virtual float4x4 view_matrix() noexcept {
            return lookAt(_position, _position + _front, _up);
        }

        [[nodiscard]] virtual float fov() noexcept {
            return _fov;
        }
    };

}