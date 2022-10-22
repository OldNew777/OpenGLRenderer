//
// Created by chenxin on 2022/9/21.
//

#pragma once

#include <nlohmann/json.hpp>

#include <core/stl.h>
#include <core/logger.h>
#include <core/constant.h>
#include <base/scene_info.h>

namespace gl_render {

    class SceneNode {
    public:
        SceneNode(nlohmann::json &json) noexcept: _json{json} {}

        [[nodiscard]] inline bool contains(string_view key) noexcept {
            return _json.contains(key);
        }

        [[nodiscard]] inline auto &operator[](string_view key) noexcept {
            return _json[key];
        }

        virtual void print() const noexcept {
            stringstream ss;
            ss << _json;
            GL_RENDER_INFO("SceneNode: {}", ss.str());
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
                return _property_vector<vector_element_t<T>, vector_dimension_v<T>>(key);
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
            if (_json.contains(key)) {
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
            if (_json.contains(key)) {
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
            _material_info = make_unique<MaterialInfo>();
            _material_info->type = MaterialInfo::String2Type(property_string("type"));
            _material_info->name = property_string("name");
            _material_info->diffuse = property_float3_or_default("diffuse", float3(0.5f));
            _material_info->specular = property_float3_or_default("specular", float3(0.f));
            _material_info->ambient = property_float3_or_default("ambient", float3(0.f));
            _material_info->diffuse_map = property_string_or_default("diffuse_map", "");
        }

        [[nodiscard]] inline auto material_info() const noexcept {
            return _material_info.get();
        }

    protected:
        unique_ptr<MaterialInfo> _material_info;

    };

    class MeshNode : public SceneNode {
    public:
        MeshNode(nlohmann::json &json) noexcept: SceneNode{json} {
            _mesh_info.file_path = property_string("file");
            _mesh_info.material_name = property_string("material");
            if (contains("transform")) {
                auto transfrom_node = SceneNode{_json["transform"]};
                if (transfrom_node.contains("matrix")) {
                    _mesh_info.transform = transfrom_node.property_float4x4_or_default(
                            "matrix",
                            constant::IDENTITY_FLOAT4x4);
                } else {
                    if (transfrom_node.contains("scale")) {
                        _mesh_info.transform = scale(
                                _mesh_info.transform,
                                transfrom_node.property_float3_or_default("scale", float3(1.f)));
                    }
                    if (transfrom_node.contains("rotate")) {
                        auto rotate_node = SceneNode{transfrom_node["rotate"]};
                        auto axis = rotate_node.property_float3("axis");
                        auto angle = rotate_node.property_float("angle");
                        _mesh_info.transform = rotate(_mesh_info.transform, angle, axis);
                    }
                    if (transfrom_node.contains("translate")) {
                        _mesh_info.transform = translate(_mesh_info.transform,
                                                         transfrom_node.property_float3("translate"));
                    }
                }
            }
        }

        [[nodiscard]] inline const auto &mesh_info() const noexcept {
            return _mesh_info;
        }

    protected:
        MeshInfo _mesh_info;
    };

    class LightNode : public SceneNode {
    public:
        LightNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            _light_info.position = property_float3("position");
            _light_info.emission = property_float3("emission");
            _light_info.emission *= property_float_or_default("scale", 1.f);
        }

        [[nodiscard]] inline const auto &light_info() const noexcept {
            return _light_info;
        }

    protected:
        LightInfo _light_info;
    };

    class CameraNode : public SceneNode {
    public:
        CameraNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            _camera_info.resolution = property_uint2("resolution");
            _camera_info.position = property_float3_or_default("position", float3{0.f});
            _camera_info.front = property_float3_or_default("front", float3{0.0f, 0.0f, -1.0f});
            _camera_info.up = property_float3_or_default("up", float3{0.0f, 1.0f, 0.0f});
            _camera_info.fov = property_float_or_default("fov", 35.f);
        }

        [[nodiscard]] inline const auto &camera_info() const noexcept {
            return _camera_info;
        }

    protected:
        CameraInfo _camera_info;
    };


    class RendererNode : public SceneNode {
    public:
        RendererNode(nlohmann::json &json) noexcept
                : SceneNode{json} {
            _renderer_info.enable_vsync = property_bool_or_default("enable_vsync", true);
            _renderer_info.enable_shadow = property_bool_or_default("enable_shadow", true);
            _renderer_info.output_file = property_string_or_default("output_file", "output.exr");
        }

        [[nodiscard]] inline const auto &renderer_info() const noexcept {
            return _renderer_info;
        }

    protected:
        RendererInfo _renderer_info;
    };

    class SceneAllNode : public SceneNode {
    public:
        SceneAllNode(nlohmann::json &json) noexcept: SceneNode{json} {
            GL_RENDER_ASSERT(json.contains("materials"), "Scene file must contain materials.");
            GL_RENDER_ASSERT(json.contains("meshes"), "Scene file must contain meshes.");
            GL_RENDER_ASSERT(json.contains("camera"), "Scene file must contain camera.");
            GL_RENDER_ASSERT(json.contains("renderer"), "Scene file must contain renderer.");

            for (auto &material_json: json["materials"]) {
                auto type = MaterialInfo::String2Type(material_json["type"]);
                unique_ptr<MaterialNode> material;
                // TODO: specify material type
                material = make_unique<MaterialNode>(material_json);
                auto material_name = material->material_info()->name;
                if (_scene_all_info.materials.contains(material_name)) {
                    GL_RENDER_ERROR_WITH_LOCATION(
                            "Material '{}' already exists.",
                            material_name);
                }
                _scene_all_info.materials.insert({
                    material_name,
                    std::forward<unique_ptr<MaterialNode>>(material)
                });
            }
            for (auto &mesh_json: json["meshes"]) {
                auto mesh_node = _scene_all_info.meshes.emplace_back(MeshNode{mesh_json});
                auto material_name = mesh_node.mesh_info().material_name;
                if (!_scene_all_info.materials.contains(material_name)) {
                    GL_RENDER_ERROR_WITH_LOCATION(
                            "Material '{}' does not exist.",
                            material_name);
                }
            }
            for (auto &light_json: json["lights"]) {
                _scene_all_info.lights.emplace_back(LightNode{light_json});
            }
            _scene_all_info.camera.emplace(CameraNode{json["camera"]});
            _scene_all_info.renderer.emplace(RendererNode{json["renderer"]});
        }

        [[nodiscard]] inline const auto &scene_all_info() const noexcept {
            return _scene_all_info;
        }

    public:
        struct SceneAllInfo {
            unordered_map<string, unique_ptr<MaterialNode>> materials;
            vector<MeshNode> meshes;
            vector<LightNode> lights;
            optional<CameraNode> camera;
            optional<RendererNode> renderer;
        };

    protected:
        SceneAllInfo _scene_all_info;
    };

}