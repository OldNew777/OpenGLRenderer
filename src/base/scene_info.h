//
// Created by ChenXin on 2022/9/21.
//

#pragma once

#include <core/stl.h>
#include <core/constant.h>
#include <core/logger.h>

namespace gl_render {

    struct SceneNodeInfo {
        virtual void print() const noexcept {
            GL_RENDER_INFO("SceneNodeInfo");
        }
    };

    class MaterialInfo : public SceneNodeInfo {
    public:
        enum class MaterialType {
            Phong,
        };

    private:
        static std::unordered_map<string, MaterialType> STRING2TYPE;
        static std::unordered_map<MaterialType, string> TYPE2STRING;

    public:
        [[nodiscard]] static inline auto String2Type(const string& type) noexcept {
            auto it = MaterialInfo::STRING2TYPE.find(type);
            if (it == STRING2TYPE.end()) {
                GL_RENDER_ERROR("Material type \"{}\" not found", type);
            }
            return it->second;
        }
        [[nodiscard]] static inline auto Type2String(MaterialType type) noexcept {
            return MaterialInfo::TYPE2STRING.find(type)->second;
        }

    public:
        string name;
        float3 diffuse;
        float3 specular;
        float3 ambient;
        path diffuse_map;
        MaterialType type;

        void print() const noexcept override {
            GL_RENDER_INFO("MaterialInfo: name = ({}), "
                           "diffuse = ({}, {}, {}), "
                           "specular = ({}, {}, {}), "
                           "ambient = ({}, {}, {}), "
                           "diffuse_map = {}",
                           name,
                           diffuse.x, diffuse.y, diffuse.z,
                           specular.x, specular.y, specular.z,
                           diffuse.x, diffuse.y, diffuse.z,
                           diffuse_map.string());
            GL_RENDER_INFO("{}", to_string(diffuse));
        }
        [[nodiscard]] virtual size_t texture_num() const noexcept {
            return 1u;
        }
    };

    struct MeshInfo : public SceneNodeInfo  {
        float4x4 transform = constant::IDENTITY_FLOAT4x4;
        path file_path;
        string material_name;

        void print() const noexcept override {
            GL_RENDER_INFO(
                    "MeshInfo: file_path: {}, material_name: {}, transform(not printed)",
                    file_path.string(), material_name);
        }
    };

    struct LightInfo : public SceneNodeInfo  {
        float3 position;
        float3 emission;

        void print() const noexcept override {
            GL_RENDER_INFO(
                    "LightInfo: position: ({}, {}, {}), emission: ({}, {}, {})",
                    position.x, position.y, position.z, emission.x, emission.y, emission.z);
        }
    };

    struct CameraInfo : public SceneNodeInfo  {
        uint2 resolution;
        float3 position;
        float3 front;
        float3 up;
        float fov;

        void print() const noexcept override {
            GL_RENDER_INFO(
                    "CameraInfo: resolution: ({}, {}), position: ({}, {}, {}), front: ({}, {}, {}), up: ({}, {}, {}), fov: {}",
                    resolution.x, resolution.y, position.x, position.y, position.z,
                    front.x, front.y, front.z, up.x, up.y, up.z, fov);
        }
    };

    struct RendererInfo : public SceneNodeInfo  {
        bool enable_vsync;
        bool enable_shadow;
        path output_file;
        uint2 shadow_map_resolution = {2048, 2048};

        void print() const noexcept override {
            GL_RENDER_INFO(
                    "RendererInfo: enable_two_sided_shading: {}, enable_shadow: {}, output_file: {}",
                    enable_vsync, enable_shadow, output_file.string());
        }
    };

}