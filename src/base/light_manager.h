//
// Created by ChenXin on 2022/10/25.
//

#pragma once

#include <base/light.h>

namespace gl_render {

    class LightManager {
    private:
        gl_render::vector<gl_render::unique_ptr<Light>> _lights;
        gl_render::vector<float3> *_vertex_positions;

    public:
        bool enable_shadow = true;

    public:
        explicit LightManager(gl_render::vector<float3> *vertex_positions) noexcept {
            _vertex_positions = vertex_positions;
        }
        ~LightManager() noexcept = default;

        void addLight(LightInfo *lightInfo, uint2 shadowResolution) noexcept {
            _lights.emplace_back(gl_render::make_unique<Light>(lightInfo, shadowResolution, _vertex_positions));
        }

        void renderShadow(float far_plane) noexcept {
            if (!enable_shadow) return;
            for (auto &light : _lights) {
                light->renderShadow(far_plane);
            }
        }

        [[nodiscard]] const auto &lights() const noexcept { return _lights; }
    };

}
