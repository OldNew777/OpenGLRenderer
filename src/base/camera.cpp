//
// Created by ChenXin on 2022/9/19.
//

#include <base/camera.h>

namespace gl_render {

    Camera::Camera(float3 position, float3 front, float3 up, float fov) :
            _position(position), _front(front), _up(up), _fov(fov) {
        _front = normalize(_front);
        _right = normalize(cross(_front, _up));
        _up = normalize(cross(_right, _front));
    }

    float4x4 Camera::getViewMatrix() noexcept {
        return lookAt(_position, _position + _front, _up);
    }

    float Camera::getFov() noexcept {
        return _fov;
    }

}