//
// Created by ChenXin on 2022/9/19.
//

#include <base/camera.h>

#include <glm/gtc/matrix_transform.hpp>

namespace gl_render {

    Camera::Camera(float3 position, float3 front, float3 up, float fov) :
            _position(position), _front(front), _up(up), _fov(fov) {
        updateCameraVectors();
    }

    void Camera::updateCameraVectors() noexcept {
        _front = normalize(_front);
        _right = normalize(cross(_front, _up));
        _up = normalize(cross(_right, _front));
    }

    float4x4 Camera::GetViewMatrix() noexcept {
        return glm::lookAt(_position, _position + _front, _up);
    }

}