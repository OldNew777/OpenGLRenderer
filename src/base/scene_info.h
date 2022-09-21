//
// Created by chenxin on 2022/9/21.
//

#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

#include <core/stl.h>
#include <core/logger.h>
#include <core/constant.h>

namespace gl_render {

    class SceneNode {
    public:
        SceneNode(nlohmann::json &json) noexcept: _json{json} {}

        [[nodiscard]] inline bool contains(string_view key) noexcept {
            return _json.contains(key);
        }
        [[nodiscard]] inline auto& operator[](string_view key) noexcept {
            return _json[key];
        }

#define GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(type)                  \
    [[nodiscard]] type property_##type(                                  \
        std::string_view name) const noexcept {                          \
        if (auto prop = _property_generic<type>(name)) [[likely]] {      \
            return *prop;                                                \
        }                                                                \
        GL_RENDER_ERROR_WITH_LOCATION(                                   \
            "No valid values given for property '{}'",                   \
            name);                                                       \
    }                                                                    \
    template<typename DV = type>                                         \
    [[nodiscard]] type property_##type##_or_default(                     \
        std::string_view name,                                           \
        DV &&default_value = std::remove_cvref_t<DV>{}) const noexcept { \
        return _property_generic<type>(name).value_or(                   \
            std::forward<DV>(default_value));                            \
    }

        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(string)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(bool)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(int)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(uint)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(uint2)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(uint3)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(uint4)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float2)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float3)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float4)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float2x2)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float3x3)
        GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER(float4x4)
#undef GL_RENDER_SCENE_NODE_DESC_PROPERTY_GETTER

    private:
        template<typename T>
        [[nodiscard]] optional<T> _property_generic(string_view key) const noexcept {
            if constexpr (is_vector_v<T>) {
                return _property_vector<vector_element_t<T> , vector_dimension_v<T>>(key);
            } else if constexpr (is_matrix_v<T>) {
                return _property_matrix<matrix_dimension_v<T>>(key);
            } else {
                return _property_scalar<T>(key);
            }
        }

        template<typename T>
        [[nodiscard]] optional<T> _property_scalar(string_view key) const noexcept {
            if (_json.contains(key)) {
                return optional<T>{_json[key].get<T>()};
            } else {
                return gl_render::nullopt;
            }
        }

        template<typename T, size_t N>
        [[nodiscard]] optional<Vector<T, N>> _property_vector(string_view key) const noexcept {
            if (_json.contains(key))  {
                auto vec_value = _json[key].get<vector<T>>();
                auto ans = Vector<T, N>{};
                for (size_t i = 0; i < N; ++i) {
                    ans[i] = vec_value[i];
                }
                return optional<Vector<T, N>>{ans};
            } else {
                return gl_render::nullopt;
            }
        }

        template<size_t N>
        [[nodiscard]] optional<Matrix<N>> _property_matrix(string_view key) const noexcept {
            if (_json.contains(key))  {
                auto vec_value = _json[key].get<vector<float>>();
                auto ans = Matrix<N>{};
                for (size_t i = 0; i < N; ++i) {
                    for (size_t j = 0; j < N; ++j) {
                        ans[i][j] = vec_value[i * N + j];
                    }
                }
                return optional<Matrix<N>>{ans};
            } else {
                return gl_render::nullopt;
            }
        }

    protected:
        nlohmann::json &_json;
    };

    class MaterialNode : public SceneNode {
    public:
        MaterialNode(nlohmann::json &json) noexcept: SceneNode{json} {
            _name = property_string("name");
            _albedo = property_float3("albedo");
        }

        friend class SceneAllNode;

    protected:
        gl_render::string _name;
        float3 _albedo;
    };

    class MeshNode : public SceneNode {
    public:
        MeshNode(nlohmann::json &json) noexcept
                : SceneNode{json},
                  _transform(constant::IDENTITY_FLOAT4x4) {
            _file_path = property_string("file");
            _material_name = property_string("material");
            if (contains("transform")) {
                auto transfrom_node = SceneNode{_json["transform"]};
                if (transfrom_node.contains("matrix")){
                    _transform = transfrom_node.property_float4x4_or_default("matrix", constant::IDENTITY_FLOAT4x4);
                } else {
                    if (transfrom_node.contains("scale")) {
                        _transform = scale(_transform, transfrom_node.property_float3_or_default("scale", float3(1.f)));
                    }
                    if (transfrom_node.contains("rotate")) {
                        auto rotate_node = SceneNode{transfrom_node["rotate"]};
                        auto axis = rotate_node.property_float3("axis");
                        auto angle = rotate_node.property_float("angle");
                        _transform = rotate(_transform, angle, axis);
                    }
                    if (transfrom_node.contains("translate")) {
                        _transform = translate(_transform, transfrom_node.property_float3("translate"));
                    }
                }
            }
        }

    protected:
        float4x4 _transform;
        std::filesystem::path _file_path;
        string _material_name;
    };

    class CameraNode : public SceneNode {
    public:
        CameraNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            _resolution = property_uint2("resolution");
            auto transform_node = SceneNode{_json["transform"]};
            _position = transform_node.property_float3_or_default("position", float3{0.f});
            _front = transform_node.property_float3_or_default("look_at", float3{0.0f, 0.0f, -1.0f}) - _position;
            _up = transform_node.property_float3_or_default("up", float3{0.0f, 1.0f, 0.0f});
            _fov = property_float_or_default("fov", 35.f);
        }

    protected:
        uint2 _resolution;
        float3 _position;
        float3 _front;
        float3 _up;
        float _fov;
    };



    class RendererNode : public SceneNode {
    public:
        RendererNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            _enable_two_sided_shading = property_bool_or_default("enable_two_sided_shading", false);
            _enable_shadow = property_bool_or_default("enable_shadow", true);
            _output_file = property_string_or_default("output_file", "output.exr");
        }

    protected:
        bool _enable_two_sided_shading;
        bool _enable_shadow;
        std::filesystem::path _output_file;
    };

    class SceneAllNode : public SceneNode {
    public:
        SceneAllNode(nlohmann::json &json) noexcept: SceneNode{json} {
            GL_RENDER_ASSERT(json.contains("materials"), "Scene file must contain materials.");
            GL_RENDER_ASSERT(json.contains("meshes"), "Scene file must contain meshes.");
            GL_RENDER_ASSERT(json.contains("camera"), "Scene file must contain camera.");
            GL_RENDER_ASSERT(json.contains("renderer"), "Scene file must contain renderer.");

            for (auto &material_json: json["materials"]) {
                auto material = MaterialNode{material_json};
                if (_materials.contains(material._name)) {
                    GL_RENDER_ERROR_WITH_LOCATION(
                            "Material '{}' already exists.",
                            material._name);
                }
                _materials.insert({material._name, material});
            }
            for (auto &mesh_json: json["meshes"]) {
                _meshes.emplace_back(MeshNode{mesh_json});
            }
            _camera.emplace(CameraNode{json["camera"]});
            _renderer.emplace(RendererNode{json["renderer"]});
        }

    protected:
        unordered_map<string, MaterialNode> _materials;
        vector<MeshNode> _meshes;
        optional<CameraNode> _camera;
        optional<RendererNode> _renderer;
    };

}