//
// Created by ChenXin on 2022/10/25.
//

#pragma once

#include <glad/glad.h>

#include <core/stl.h>
#include <core/logger.h>
#include <base/scene_info.h>
#include <base/depth_cube_map.h>

namespace gl_render {

    class Light {
    private:
        const LightInfo *_lightInfo;
        uint2 _shadowResolution;
        gl_render::vector<float4x4>_shadowTransforms;
        gl_render::unique_ptr<DepthCubeMap> _depthCubeMap;
        float _far_plane;

    public:
        Light(const LightInfo *lightInfo, uint2 shadowResolution,
                gl_render::vector<float3> *vertex_positions) noexcept
                : _lightInfo(lightInfo), _shadowResolution(shadowResolution) {
            _shadowTransforms.resize(6);
            _depthCubeMap = gl_render::make_unique<DepthCubeMap>(_shadowResolution, vertex_positions);
        }

        ~Light() noexcept = default;

        void renderShadow(float far_plane) noexcept {
            _far_plane = far_plane;

            // initialize shadow transforms
            float4x4 shadowProj = perspective(
                    radians(90.0f),
                    (float)_shadowResolution.x / (float)_shadowResolution.y,
                    0.02f,
                    far_plane);
            _shadowTransforms[0] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            _shadowTransforms[1] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            _shadowTransforms[2] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
            _shadowTransforms[3] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
            _shadowTransforms[4] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            _shadowTransforms[5] = shadowProj * lookAt(_lightInfo->position, _lightInfo->position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));

            _depthCubeMap->render(far_plane, _lightInfo->position, _shadowTransforms);
        }

        [[nodiscard]] auto *lightInfo() const noexcept { return _lightInfo; }
        [[nodiscard]] auto depthCubeMapHandle() const noexcept { return _depthCubeMap->depthCubeMapHandle(); }
        [[nodiscard]] auto far_plane() const noexcept { return _far_plane; }

    };

}