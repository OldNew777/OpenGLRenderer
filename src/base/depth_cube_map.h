//
// Created by ChenXin on 2022/10/25.
//

#pragma once

#include <glad/glad.h>

#include <core/stl.h>
#include <core/logger.h>
#include <base/shader.h>

namespace gl_render {

    class DepthCubeMap {
    private:
        uint2 _shadowResolution;
        GLuint _depthCubeMapFBO{0u};
        GLuint _depthCubeMap{0u};
        GLuint64 _depthCubeMapHandle{0u};

        static GLuint VERTEX_ARRAY;
        static GLuint POSITION_BUFFER;
        static int TRIANGLE_COUNT;
        static gl_render::unique_ptr<Shader> SHADER;
        static int INSTANCE_NUM;

    public:
        void render(float far_plane, const float3 &lightPos,
                    const vector <float4x4> &shadowTransforms) const noexcept;

        explicit DepthCubeMap(uint2 shadowResolution,
                              gl_render::vector<float3> *vertex_positions) noexcept;

        ~DepthCubeMap() noexcept;

        [[nodiscard]] inline auto depthCubeMapHandle() const noexcept { return _depthCubeMapHandle; }

    };

}