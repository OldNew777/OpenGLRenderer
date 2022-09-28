//
// Created by ChenXin on 2022/9/21.
//

#pragma once

#include <base/scene_parser.h>
#include <base/shader.h>

namespace gl_render {

    namespace impl {

        struct AABB {
            float3 min{1.e6f};
            float3 max{-1.e6f};
        };

    }

    class Geometry {

    public:
        using AABB = impl::AABB;

    private:
        vector<size_t> _mesh_offsets;
        vector<size_t> _mesh_sizes;
        AABB _aabb{};
        size_t _triangle_count{0};
        size_t _vertex_count{0};
        size_t _texture_count{0};
        uint _vertex_array{0};
        uint _position_buffer{0};
        uint _normal_buffer{0};
        uint _color_buffer{0};
        uint _tex_coord_buffer{0};
        uint _tex_property_buffer{0};
        uint _texture_array{0};

    private:
        template<typename T>
        static T _flatten(const T &v, const std::vector<uint3> &indices) noexcept {
            T container;
            container.reserve(indices.size() * 3);
            for (auto i : indices) {
                container.emplace_back(v[i.x]);
                container.emplace_back(v[i.y]);
                container.emplace_back(v[i.z]);
            }
            return container;
        }

    public:
        explicit Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo);

        ~Geometry();
        Geometry(Geometry &&) = default;
        Geometry(const Geometry &) = delete;
        Geometry &operator=(Geometry &&) = default;
        Geometry &operator=(const Geometry &) = delete;

        [[nodiscard]] AABB aabb() const noexcept { return _aabb; }
        [[nodiscard]] const std::vector<size_t> &mesh_offsets() const noexcept { return _mesh_offsets; }
        [[nodiscard]] const std::vector<size_t> &mesh_sizes() const noexcept { return _mesh_sizes; }
        [[nodiscard]] uint position_buffer_id() const noexcept { return _position_buffer; }
        [[nodiscard]] uint normal_buffer_id() const noexcept { return _normal_buffer; }
        [[nodiscard]] size_t texture_count() const noexcept { return _texture_count; }
        void render(const Shader &shader) const;
        void shadow(const Shader &shader) const;
    };

}