//
// Created by ChenXin on 2022/9/21.
//

#pragma once

#include <base/scene_parser.h>
#include <base/shader.h>
#include <base/light_manager.h>

#include <assimp/scene.h>

namespace gl_render {

    namespace impl {

        struct AABB {
            float3 min{1.e10f};
            float3 max{-1.e10f};

            [[nodiscard]] auto &operator[](size_t index) noexcept {
                switch (index) {
                    case 0:
                        return min;
                    case 1:
                        return max;
                    default:
                    GL_RENDER_ERROR("AABB index out of range");
                }
            }

            [[nodiscard]] const auto &operator[](size_t index) const noexcept {
                switch (index) {
                    case 0:
                        return min;
                    case 1:
                        return max;
                    default:
                    GL_RENDER_ERROR("AABB index out of range");
                }
            }
        };

        template<typename T>
        static T _flatten(const T &v, const std::vector<uint3> &indices) noexcept {
            T container;
            container.reserve(indices.size() * 3u);
            for (auto i: indices) {
                container.emplace_back(v[i.x]);
                container.emplace_back(v[i.y]);
                container.emplace_back(v[i.z]);
            }
            return container;
        }

        struct MeshInfoGrouped {
            aiMesh *ai_mesh;
            const MeshInfo *mesh_info;
            uint offset;
        };

        struct MeshSqueezed {
            gl_render::vector<float3> vertices;
            gl_render::vector<float3> normals;
        };

    }

    class GeometryGroup {

    private:
        impl::AABB _aabb;
        unique_ptr<Shader> _shader;
        uint _buffer_num;
        uint _texture_num;
        uint _triangle_count{0u};

        GLuint _vertex_array{0u};
        GLuint _position_buffer{0u};
        GLuint _normal_buffer{0u};

        GLuint _diffuse_buffer{0u};
        GLuint _specular_buffer{0u};
        GLuint _ambient_buffer{0u};
        GLuint _tex_coord_buffer{0u};
        vector<GLuint64> _texture_handles;

    public:
        GeometryGroup(MaterialInfo* material, const gl_render::vector<impl::MeshInfoGrouped>& meshInfoGroupedVec,
                      const path &scene_dir, Shader::TemplateList tl = {}) noexcept;
        ~GeometryGroup() noexcept;

        GeometryGroup(GeometryGroup &&) = delete;
        GeometryGroup(const GeometryGroup &) = delete;
        GeometryGroup &operator=(GeometryGroup &&) = delete;
        GeometryGroup &operator=(const GeometryGroup &) = delete;

        virtual void render() const;
        virtual void shadow() const;
        void set_lights(LightManager *lightManager) const;
        void set_camera(
                const float4x4& projection,
                const float4x4& view,
                const float3& cameraPos) const;

        [[nodiscard]] Shader* shader() const noexcept { return _shader.get(); }
        [[nodiscard]] auto aabb() const noexcept { return _aabb; }
        [[nodiscard]] auto position_buffer() const noexcept { return _position_buffer; }
        [[nodiscard]] auto triangle_count() const noexcept { return _triangle_count; }
    };

    class Geometry {

    private:
        impl::AABB _aabb;
        vector<unique_ptr<GeometryGroup>> _groups;
        vector<float3> _vertex_positions_flattened;

    public:
        explicit Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo, const path &scene_dir);

        ~Geometry() = default;
        Geometry(Geometry &&) = delete;
        Geometry(const Geometry &) = delete;
        Geometry &operator=(Geometry &&) = delete;
        Geometry &operator=(const Geometry &) = delete;

        [[nodiscard]] auto aabb() const noexcept { return _aabb; }

    public:
        void render(
                LightManager *lightManager,
                const float4x4& projection,
                const float4x4& view,
                const float3& cameraPos) const;
        void shadow(
                LightManager *lightManager,
                const float4x4& projection,
                const float4x4& view,
                const float3& cameraPos) const;
        [[nodiscard]] auto vertex_positions_flattened() noexcept;

    };

}