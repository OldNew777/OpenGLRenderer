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
            if constexpr (is_vector_v<T>()) {
//                return _property_vector<vector_element_t<T> , vector_dimension_v<T>>(key);
                GL_RENDER_ERROR_WITH_LOCATION("Not implemented.");
            } else if constexpr (is_matrix_v<T>()) {
                GL_RENDER_ERROR_WITH_LOCATION("Not implemented.");
            } else {
                return _property_scalar<T>(key);
            }
        }

        template<typename T>
        [[nodiscard]] optional<T> _property_scalar(string_view key) const noexcept {
            if (_json.contains(key)) {
                return optional<T>{_json[key].get<T>()};
            } else {
                return optional<T>{};
            }
        }

        template<typename T>
        [[nodiscard]] optional<T> _property_vector(string_view key) const noexcept {
            GL_RENDER_ERROR_WITH_LOCATION("Not implemented.");
        }

        template<typename T>
        [[nodiscard]] optional<T> _property_matrix(string_view key) const noexcept {
            GL_RENDER_ERROR_WITH_LOCATION("Not implemented.");
        }

    protected:
        nlohmann::json &_json;
    };

    class MaterialNode : public SceneNode {
    public:
        MaterialNode(nlohmann::json &json) noexcept: SceneNode{json} {
            _name = property_string("name");
            // TODO: change to property_*
            auto albedo_vec = json["albedo"].get<vector<float>>();
            _albedo = float3{albedo_vec[0], albedo_vec[1], albedo_vec[2]};
        }

        friend class SceneAllNode;

    private:
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
        }

    private:
        float4x4 _transform;
        std::filesystem::path _file_path;
        string _material_name;
    };

    class CameraNode : public SceneNode {
    public:
        CameraNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            // TODO: change to property_*
            auto position_vec = json["transform"]["position"].get<vector<float>>();
            _position = float3{position_vec[0], position_vec[1], position_vec[2]};
            auto look_at_vec = json["transform"]["look_at"].get<vector<float>>();
            _front = float3{look_at_vec[0], look_at_vec[1], look_at_vec[2]} - _position;
            auto up_vec = json["transform"]["up"].get<vector<float>>();
            _up = float3{up_vec[0], up_vec[1], up_vec[2]};
            _fov = property_float("fov");
        }

    private:
        float3 _position;
        float3 _front;
        float3 _up;
        float _fov;
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
        }

    private:
        unordered_map<string, MaterialNode> _materials;
        vector<MeshNode> _meshes;
        optional<CameraNode> _camera;
    };

}