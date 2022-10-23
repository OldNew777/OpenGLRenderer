//
// Created by ChenXin on 2022/9/21.
//

#include <base/geometry.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <base/texture_packer.h>

namespace gl_render {

    Geometry::Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo, const path &scene_dir) {
        for (auto &&mesh_node: sceneAllInfo.meshes) {
            const auto &mesh = mesh_node.mesh_info();

            auto mesh_path = mesh.file_path.string();
            if (mesh.file_path.is_relative()) {
                mesh_path = (scene_dir / mesh.file_path).string();
            }
            auto material_name = mesh.material_name;

            Assimp::Importer importer;
            auto ai_scene = importer.ReadFile(
                    mesh_path,
                    aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_GenNormals |
                    aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
            GL_RENDER_ASSERT(ai_scene != nullptr, "Mesh \"{}\" is nullptr", mesh_path);
            GL_RENDER_ASSERT(!(ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Mesh \"{}\" is incomplete", mesh_path);
            GL_RENDER_ASSERT(ai_scene->mRootNode != nullptr, "Failed to load mesh: {}", mesh_path);


            // gather submeshes
            vector<aiMesh *> mesh_list;
            queue<aiNode *> node_queue;
            node_queue.push(ai_scene->mRootNode);
            while (!node_queue.empty()) {
                auto node = node_queue.front();
                node_queue.pop();
                for (auto i = 0ul; i < node->mNumMeshes; i++) {
                    mesh_list.emplace_back(ai_scene->mMeshes[node->mMeshes[i]]);
                }
                for (auto i = 0ul; i < node->mNumChildren; i++) {
                    node_queue.push(node->mChildren[i]);
                }
            }

            GL_RENDER_INFO("Loaded mesh \"{}\", list size: {}", mesh_path, mesh_list.size());

            // process submeshes
            for (auto ai_mesh: mesh_list) {
                // process material
                auto iter = sceneAllInfo.materials.find(material_name);
                if (iter == sceneAllInfo.materials.end()) {
                    GL_RENDER_ERROR_WITH_LOCATION("Reference to undefined material: {}", material_name);
                }
                auto material = iter->second->material_info();

                Shader::TemplateList tl = {
                        {std::string{"POINT_LIGHT_COUNT"}, serialize(sceneAllInfo.lights.size())}
                };
                _groups.emplace_back(make_unique<GeometryGroup>(
                        mesh, material, ai_mesh, scene_dir,
                        tl));
                _aabb.min = min(_aabb.min, _groups.back()->aabb().min);
                _aabb.max = max(_aabb.max, _groups.back()->aabb().max);
            }

            GL_RENDER_INFO(
                    "All meshes AABB: min = {}, max = {})",
                    to_string(_aabb.min),
                    to_string(_aabb.max));
            GL_RENDER_INFO("Group count: {}", _groups.size());
        }
    }

    void Geometry::render(
            const vector<LightNode> &lights,
            const float4x4& projection,
            const float4x4& view,
            const float3& cameraPos) const {
        for (auto &group: _groups) {
            auto shader = group->shader();
            shader->use();
            group->set_lights(lights);
            group->set_camera(projection, view, cameraPos);
            group->render();
        }
    }

    void Geometry::shadow(
            const vector<LightNode> &lights,
            const float4x4& projection,
            const float4x4& view,
            const float3& cameraPos) const {
        for (auto &group: _groups) {
            auto shader = group->shader();
            shader->use();
            group->set_lights(lights);
            group->set_camera(projection, view, cameraPos);
            group->shadow();
        }
    }

    GeometryGroup::GeometryGroup(const MeshInfo &mesh, MaterialInfo *material,
                                 aiMesh *ai_mesh, const path &scene_dir,
                                 const Shader::TemplateList &tl) noexcept {
        buffer_num = 7;
        texture_num = 1;
        string type_string = MaterialInfo::Type2String(material->type);
        _shader = make_unique<Shader>(
                "data/shaders/" + type_string + ".vert",
                "data/shaders/" + type_string + ".frag",
                "",
                tl
        );
        _shader->use();

        vector<float3> positions;
        vector<float3> normals;
        vector<float3> diffuse_vec;
        vector<float3> specular_vec;
        vector<float3> ambient_vec;
        vector<float3> tex_coords;
        vector<float4> tex_properties;
        vector<uint3> indices;

        TexturePacker packer;

        // process material
        float3 diffuse{1.0f};
        auto has_texture = true;
        if (material->diffuse_map.empty()) {
            diffuse = material->diffuse;
            has_texture = false;
        }

        TexturePacker::ImageBlock block{};
        if (has_texture) {
            auto diffuse_map_path = material->diffuse_map;
            if (material->diffuse_map.is_relative()) {
                diffuse_map_path = (scene_dir / material->diffuse_map).string();
            }
            block = packer.load(diffuse_map_path);
        }

        auto model_matrix = mesh.transform;
        auto normal_matrix = transpose(inverse(model_matrix));

        // process vertices
        for (auto i = 0ul; i < ai_mesh->mNumVertices; i++) {
            auto ai_position = ai_mesh->mVertices[i];
            auto ai_normal = ai_mesh->mNormals[i];
            auto temp = model_matrix * float4{ai_position.x, ai_position.y, ai_position.z, 1.f};
            auto position = float3(temp.x, temp.y, temp.z);
            auto normal = normal_matrix * float4{ai_normal.x, ai_normal.y, ai_normal.z, 1.f};
            positions.emplace_back(position);
            normals.emplace_back(normal);
            _aabb.min = min(_aabb.min, position);
            _aabb.max = max(_aabb.max, position);

            // TODO: move properties of phong .etc to children class
            auto ai_tex_coords = ai_mesh->mTextureCoords[0];
            if (!has_texture || ai_tex_coords == nullptr) {
                tex_coords.emplace_back(0.0f, 0.0f, -1.0f);
                tex_properties.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
            } else {
                tex_coords.emplace_back(ai_tex_coords[i].x, ai_tex_coords[i].y, block.index);
                GL_RENDER_INFO("tex coord: ({}, {}, {}), tex uv: ({}, {}), tex id: {}",
                               ai_position.x, ai_position.y, ai_position.z,
                               ai_tex_coords[i].x, ai_tex_coords[i].y, block.index);
                tex_properties.emplace_back(block.offset.x, block.offset.y, block.size.x, block.size.y);
            }
            diffuse_vec.emplace_back(diffuse);
            specular_vec.emplace_back(material->specular);
            ambient_vec.emplace_back(material->ambient);
        }

        // process faces
        for (auto i = 0ul; i < ai_mesh->mNumFaces; i++) {
            auto &&face = ai_mesh->mFaces[i].mIndices;
            indices.emplace_back(uint3{face[0], face[1], face[2]});
        }

        _texture_array = packer.create_opengl_texture_array();
        _texture_max_size = packer.max_size();

        GL_RENDER_INFO(
                "AABB: min = {}, max = {})",
                to_string(_aabb.min),
                to_string(_aabb.max));

        _triangle_count = indices.size();
        auto vertex_count = _triangle_count * 3;

        // transfer to OpenGL
        glGenVertexArrays(1, &_vertex_array);

        vector<uint32_t> buffers(buffer_num);
        glGenBuffers(buffer_num, buffers.data());

        _position_buffer = buffers[0];
        _normal_buffer = buffers[1];
        _diffuse_buffer = buffers[2];
        _tex_coord_buffer = buffers[3];
        _tex_property_buffer = buffers[4];
        _specular_buffer = buffers[5];
        _ambient_buffer = buffers[6];

        // VAO
        glBindVertexArray(_vertex_array);

        glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(positions, indices).data(),
                     GL_DYNAMIC_COPY);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(normals, indices).data(),
                     GL_DYNAMIC_COPY);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _diffuse_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(diffuse_vec, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _tex_coord_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(tex_coords, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _tex_property_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float4),
                     impl::_flatten(tex_properties, indices).data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float4), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _specular_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(specular_vec, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _ambient_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(ambient_vec, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindVertexArray(0);
    }

    GeometryGroup::~GeometryGroup() noexcept {
        glDeleteVertexArrays(1, &_vertex_array);
        glDeleteBuffers(buffer_num, &_position_buffer);
        glDeleteTextures(texture_num, &_texture_array);
    }

    void GeometryGroup::render() const {
        glBindVertexArray(_vertex_array);
        // FIXME: texture rendering error now
        glActiveTexture(GL_TEXTURE0);
        _shader->setInt("textures", 0);
        _shader->setUint("TEXTURE_MAX_SIZE", _texture_max_size);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_array);
        glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
        glBindVertexArray(0);
    }

    void GeometryGroup::shadow() const {
        glBindVertexArray(_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
        glBindVertexArray(0);
    }


    void GeometryGroup::set_lights(const vector<LightNode> &lights) const {
        // point lights
        for (auto i = 0ul; i < lights.size(); i++) {
            _shader->setVec3(serialize("pointLights[", i, "].Position"), lights[i].light_info().position);
            _shader->setVec3(serialize("pointLights[", i, "].Color"), lights[i].light_info().emission);
        }
    }

    void GeometryGroup::set_camera(const float4x4 &projection, const float4x4 &view, const float3 &cameraPos) const {
        _shader->setMat4("projection", projection);
        _shader->setMat4("view", view);
        _shader->setVec3("cameraPos", cameraPos);
    }

}
