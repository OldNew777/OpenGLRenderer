//
// Created by ChenXin on 2022/9/21.
//

#pragma once

#include <base/scene_parser.h>
#include <base/shader.h>

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

    }

    class GeometryGroup {

    private:
        impl::AABB _aabb;
        unique_ptr<Shader> _shader;
        uint buffer_num;
        uint texture_num;

        uint _triangle_count;
        uint _vertex_array;
        uint _position_buffer;
        uint _normal_buffer;

        uint _diffuse_buffer;
        uint _specular_buffer;
        uint _ambient_buffer;
        uint _tex_coord_buffer;
        uint _tex_property_buffer;
        uint _texture_array;
        uint _texture_max_size;

    public:
        GeometryGroup(const MeshInfo& mesh, MaterialInfo* material,
                      aiMesh* ai_mesh, const path &scene_dir,
                      const Shader::TemplateList &tl = {}) noexcept;
        ~GeometryGroup() noexcept;
        virtual void render() const noexcept;
        virtual void shadow() const noexcept;
        [[nodiscard]] Shader* shader() const noexcept { return _shader.get(); }
        [[nodiscard]] auto texture_max_size() const noexcept { return _texture_max_size; }

    };

    class Geometry {

    public:
        using AABB = impl::AABB;

    private:
        vector<size_t> _mesh_offsets;
        vector<size_t> _mesh_sizes;
        AABB _aabb{};

        uint _triangle_count;
        uint _vertex_array;
        uint _position_buffer;
        uint _normal_buffer;
        uint _diffuse_buffer;
        uint _specular_buffer;
        uint _ambient_buffer;
        uint _tex_coord_buffer;
        uint _tex_property_buffer;
        uint _texture_array;
        uint _texture_max_size;

    public:
        explicit Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo, const path &scene_dir);

        ~Geometry();

        Geometry(Geometry &&) = default;

        Geometry(const Geometry &) = delete;

        Geometry &operator=(Geometry &&) = default;

        Geometry &operator=(const Geometry &) = delete;

        [[nodiscard]] auto aabb() const noexcept { return _aabb; }

        [[nodiscard]] auto texture_max_size() const noexcept { return _texture_max_size; }

    public:
        void render(const Shader &shader) const;

        void shadow(const Shader &shader) const;
    };

}