//
// Created by ChenXin on 2022/10/25.
//

#pragma once

#include <glad/glad.h>
#include <core/stl.h>
#include <base/shader.h>
#include <util/imageio.h>

namespace gl_render {

    class HDR2LDR {
    public:
        explicit HDR2LDR(uint2 resolution, GLuint& hdr_frame_buffer) {
            _hdr_frame_buffer = hdr_frame_buffer;
            _shader = make_unique<Shader>(
                    "data/shaders/hdr2ldr.vert",
                    "",
                    "data/shaders/hdr2ldr.frag",
                    Shader::TemplateList{});
            // configure floating point framebuffer
            auto width = resolution.x;
            auto height = resolution.y;
            glGenFramebuffers(1, &hdr_frame_buffer);
            // create floating point color buffer
            glGenTextures(1, &_hdr_tex_buffer);
            glBindTexture(GL_TEXTURE_2D, _hdr_tex_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // create depth buffer (renderbuffer)
            uint rboDepth;
            glGenRenderbuffers(1, &rboDepth);
            glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            // attach buffers
            glBindFramebuffer(GL_FRAMEBUFFER, hdr_frame_buffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _hdr_tex_buffer, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                GL_RENDER_ERROR("Framebuffer not complete!");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        ~HDR2LDR() {
            glDeleteFramebuffers(1, &_hdr_frame_buffer);
            glDeleteTextures(1, &_hdr_tex_buffer);
        }

        void render(const HDRConfig& hdrConfig) noexcept {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _shader->use();
            _shader->setFloat("exposure", hdrConfig.exposure);
            _shader->setFloat("gamma", hdrConfig.gamma);
            static uint quadVAO = 0u;
            static uint quadVBO = 0u;
            // render Quad
            if (quadVAO == 0u) {
                static float quadVertices[] = {
                        // positions        // texture Coords
                        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                };
                // setup plane VAO
                glGenVertexArrays(1, &quadVAO);
                glGenBuffers(1, &quadVBO);
                glBindVertexArray(quadVAO);
                glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
            }
            glBindVertexArray(quadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _hdr_tex_buffer);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }

    private:
        gl_render::unique_ptr<Shader> _shader;
        GLuint _hdr_frame_buffer{0u};
        GLuint _hdr_tex_buffer{0u};
    };

}