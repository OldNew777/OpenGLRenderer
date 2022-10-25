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
                    const vector <float4x4> &shadowTransforms) noexcept {
            // 1. render scene to depth cubemap
            // --------------------------------
            glViewport(0, 0, _shadowResolution.x, _shadowResolution.y);
            glBindFramebuffer(GL_FRAMEBUFFER, _depthCubeMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            SHADER->use();
            for (unsigned int i = 0; i < 6; ++i)
                SHADER->setMat4("shadowTransforms[" + std::to_string(i) + "]", shadowTransforms[i]);
            SHADER->setFloat("far_plane", far_plane);
            SHADER->setVec3("lightPos", lightPos);

            // render scene from light's point of view
            glBindVertexArray(VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_COUNT * 3);
            glBindVertexArray(0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        explicit DepthCubeMap(uint2 shadowResolution,
                              gl_render::vector<float3> *vertex_positions) noexcept
                : _shadowResolution(shadowResolution) {
            if (INSTANCE_NUM == 0) {
                // initialize shader
                SHADER = gl_render::make_unique<Shader>(
                        path{"data/shaders/point_shadows_depth.vert"},
                        path{"data/shaders/point_shadows_depth.geom"},
                        path{"data/shaders/point_shadows_depth.frag"},
                        Shader::TemplateList{});

                // init geometry
                TRIANGLE_COUNT = vertex_positions->size() / 3;
                glGenVertexArrays(1, &VERTEX_ARRAY);

                glGenBuffers(1, &POSITION_BUFFER);
                glBindBuffer(GL_ARRAY_BUFFER, POSITION_BUFFER);
                glBufferData(GL_ARRAY_BUFFER, vertex_positions->size() * sizeof(float3), vertex_positions->data(), GL_DYNAMIC_COPY);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

                glBindVertexArray(0);
            }
            ++INSTANCE_NUM;

            // configure depth map FBO
            // -----------------------
            glGenFramebuffers(1, &_depthCubeMapFBO);
            // create depth cubemap texture
            glGenTextures(1, &_depthCubeMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, _depthCubeMap);
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                             _shadowResolution.x, _shadowResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            _depthCubeMapHandle = glGetTextureHandleARB(_depthCubeMap);
            glMakeTextureHandleResidentARB(_depthCubeMapHandle);
            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, _depthCubeMapFBO);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthCubeMap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                GL_RENDER_ERROR("DepthCubeMap framebuffer error");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        ~DepthCubeMap() noexcept {
            --INSTANCE_NUM;
            if (INSTANCE_NUM == 0) {
                glDeleteVertexArrays(1, &VERTEX_ARRAY);
                glDeleteBuffers(1, &POSITION_BUFFER);
            }
            glDeleteFramebuffers(1, &_depthCubeMapFBO);
            glDeleteTextures(1, &_depthCubeMap);
        }

        [[nodiscard]] inline auto depthMapFBO() const noexcept { return _depthCubeMapFBO; }
        [[nodiscard]] inline auto depthCubeMap() const noexcept { return _depthCubeMap; }
        [[nodiscard]] inline auto depthCubeMapHandle() const noexcept { return _depthCubeMapHandle; }

    };

}