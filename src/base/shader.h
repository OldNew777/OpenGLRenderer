//
// Created by chenxin on 2022/9/21.
//


#pragma once

#include <fstream>
#include <iostream>
#include <regex>

#include <glad/glad.h>
#include <glsl/glsl_optimizer.h>

#include <core/serialize.h>
#include <core/logger.h>

namespace gl_render {

    class Shader {
    public:

        using TemplateList = std::unordered_map<string, string>;

        unsigned int ID;

        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        Shader(const path &vertexPath, const path &fragmentPath, const string &geometryPath = {},
               const TemplateList &tl = {}) {

            // 1. retrieve the vertex/fragment source code from filePath
            string vertexCode;
            string fragmentCode;
            string geometryCode;

            vertexCode = optimizeShaderSource(readSourceFile(vertexPath, tl), kGlslOptShaderVertex);
            fragmentCode = optimizeShaderSource(readSourceFile(fragmentPath, tl), kGlslOptShaderFragment);

            // if geometry shader path is present, also load a geometry shader
            if (!geometryPath.empty()) {
                geometryCode = readSourceFile(geometryPath, tl);
            }

            const char *vShaderCode = vertexCode.c_str();
            const char *fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if (!geometryPath.empty()) {
                const char *gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if (!geometryPath.empty()){
                glAttachShader(ID, geometry);
            }
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
            // delete the shaders because they are linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (!geometryPath.empty()){
                glDeleteShader(geometry);
            }
        }

        // activate the shader
        // ------------------------------------------------------------------------
        void use() {
            glUseProgram(ID);
        }

        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const string &name, bool value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
        }

        // ------------------------------------------------------------------------
        void setInt(const string &name, int value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }

        // ------------------------------------------------------------------------
        void setFloat(const string &name, float value) const {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }

        // ------------------------------------------------------------------------
        void setVec2(const string &name, const float2 &value) const {
            glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec2(const string &name, float x, float y) const {
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }

        // ------------------------------------------------------------------------
        void setVec3(const string &name, const float3 &value) const {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec3(const string &name, float x, float y, float z) const {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }

        // ------------------------------------------------------------------------
        void setVec4(const string &name, const float4 &value) const {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec4(const string &name, float x, float y, float z, float w) {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }

        // ------------------------------------------------------------------------
        void setMat2(const string &name, const float2x2 &mat) const {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        // ------------------------------------------------------------------------
        void setMat3(const string &name, const float3x3 &mat) const {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        // ------------------------------------------------------------------------
        void setMat4(const string &name, const float4x4 &mat) const {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

    private:

        static string replaceVersionString(string &src, string_view replacement) {
            static std::regex version_string_finder{R"(#version\s+\d{3}\s+(core|es)?)",
                                                    std::regex::optimize | std::regex::ECMAScript};
            std::smatch match_result{};
            if (!std::regex_search(src, match_result, version_string_finder)) {
                throw std::runtime_error{"Failed to find version string in shader source"};
            }
            auto version_string = match_result.str();
            src = serialize(
                    string_view{src}.substr(0, match_result.position()),
                    replacement,
                    string_view{src}.substr(match_result.position() + match_result.length()));
            return version_string;
        }

        static string optimizeShaderSource(string src, glslopt_shader_type shader_type) {

            auto version_string = replaceVersionString(src, "#version 300 es");

            static constexpr auto context_deleter = [](glslopt_ctx *ctx) noexcept { glslopt_cleanup(ctx); };
            static unique_ptr<glslopt_ctx, decltype(context_deleter)> optimizer_context{
                    glslopt_initialize(kGlslTargetOpenGL), context_deleter};

            static constexpr auto shader_deleter = [](glslopt_shader *shader) noexcept {
                glslopt_shader_delete(shader);
            };
            unique_ptr<glslopt_shader, decltype(shader_deleter)> shader{
                    glslopt_optimize(optimizer_context.get(), shader_type, src.c_str(), 0), shader_deleter};

            if (!glslopt_get_status(shader.get())) {
                throw std::runtime_error{serialize("Failed to optimize shader: ", glslopt_get_log(shader.get()))};
            }
            src = glslopt_get_output(shader.get());
            replaceVersionString(src, version_string);
            return src;
        }

        static string readSourceFile(const path &path, const TemplateList &tl) {
            std::ifstream file{path};
            if (!file.is_open()) {
                GL_RENDER_ERROR_WITH_LOCATION("Failed to read file: {}", path.string());
            }

            string source;
            string templ;
            string line;
            while (!file.eof()) {
                auto c = file.get();
                if (c == '$') {  // begin of template
                    if (file.get() != '{') {
                        GL_RENDER_ERROR_WITH_LOCATION("Expected '{}' in shader template: {}", '{', path.string());
                    }
                    templ.clear();
                    while (!file.eof()) {
                        templ.push_back(file.get());
                        if (templ.back() == '}') {
                            break;
                        }
                    }
                    if (templ.back() != '}') {
                        GL_RENDER_ERROR_WITH_LOCATION("Expected '{}' in shader template: {}", '}', path.string());
                    }
                    templ.pop_back();
                    auto iter = tl.find(templ);
                    if (iter == tl.end()) {
                        GL_RENDER_ERROR_WITH_LOCATION("Unknown template name \"{}\": {}", '}', templ, path.string());
                    }
                    source.append(iter->second);
                } else if (c != EOF) {
                    source.push_back(c);
                }
            }
            return source;
        }

        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        static void checkCompileErrors(GLuint shader, const string &type) {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                    GL_RENDER_ERROR_WITH_LOCATION(
                            "ERROR::SHADER_COMPILATION_ERROR of type: {}. InfoLog: {}",
                            type, infoLog);
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                    GL_RENDER_ERROR_WITH_LOCATION(
                            "ERROR::SHADER_COMPILATION_ERROR of type: {}. InfoLog: {}",
                            type, infoLog);
                }
            }
        }
    };

}