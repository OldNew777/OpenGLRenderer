//
// Created by ChenXin on 2022/9/21.
//

#include <base/geometry.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <base/texture.h>

namespace gl_render {

    Geometry::Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo, const path &scene_dir) {
        Shader::TemplateList tl = {
                {std::string{"POINT_LIGHT_COUNT"}, serialize(sceneAllInfo.lights.size())}
        };
        gl_render::unordered_map<MaterialInfo *, gl_render::vector<impl::MeshInfoGrouped>> mesh_map;
        // maintain ownership of importers, and their scenes
        gl_render::vector<Assimp::Importer> importer(sceneAllInfo.meshes.size());
        for (auto index = 0u; index < sceneAllInfo.meshes.size(); ++index) {
            const auto &mesh_node = sceneAllInfo.meshes[index];
            const auto &mesh = mesh_node.mesh_info();

            auto mesh_path = mesh.file_path.string();
            if (mesh.file_path.is_relative()) {
                mesh_path = (scene_dir / mesh.file_path).string();
            }
            auto material_name = mesh.material_name;

            auto ai_scene = importer[index].ReadFile(
                    mesh_path,
                    aiProcess_Triangulate |
                    aiProcess_FixInfacingNormals | aiProcess_GenNormals |
                    aiProcess_RemoveRedundantMaterials |
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
                uint offset = 0u;
                if (mesh_map.find(material) != mesh_map.end()) {
                    offset = mesh_map[material].back().offset + mesh_map[material].back().ai_mesh->mNumVertices;
                }
                mesh_map[material].emplace_back(impl::MeshInfoGrouped{material, ai_mesh, &mesh, offset, scene_dir});
            }
        }

        for (auto &[material, mesh_info_grouped_vec]: mesh_map) {
            _groups.emplace_back(make_unique<GeometryGroup>(mesh_info_grouped_vec, tl));
            _aabb.min = min(_aabb.min, _groups.back()->aabb().min);
            _aabb.max = max(_aabb.max, _groups.back()->aabb().max);
        }

        GL_RENDER_INFO(
                "All meshes AABB: min = {}, max = {})",
                to_string(_aabb.min),
                to_string(_aabb.max));
        GL_RENDER_INFO("Group count: {}", _groups.size());
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

    GeometryGroup::GeometryGroup(const gl_render::vector<impl::MeshInfoGrouped>& meshInfoGroupedVec,
                                 Shader::TemplateList tl) noexcept {

        _buffer_num = 7;
        _texture_num = meshInfoGroupedVec[0].material_info->texture_num();
        string type_string = MaterialInfo::Type2String(meshInfoGroupedVec[0].material_info->type);
        tl["TEXTURE_COUNT"] = serialize(_texture_num);
        _shader = make_unique<Shader>(
                "data/shaders/" + type_string + ".vert",
                "data/shaders/" + type_string + ".frag",
                "",
                tl
        );

        vector<float3> positions;
        vector<float3> normals;
        vector<float3> diffuse_vec;
        vector<float3> specular_vec;
        vector<float3> ambient_vec;
        vector<float3> tex_coords;
        vector<GLuint64> diffuse_tex_handles;
        vector<uint3> indices;

        for (const auto& meshInfoGrouped : meshInfoGroupedVec) {
            auto material = meshInfoGrouped.material_info;
            auto scene_dir = meshInfoGrouped.scene_dir;
            GLuint64 diffuse_tex_handle = 0u;

            // process material
            float3 diffuse{0.5f, 0.f, 0.5f};
            auto has_diffuse_texture = true;
            if (material->diffuse_map.empty()) {
                diffuse = material->diffuse;
                has_diffuse_texture = false;
            }

            if (has_diffuse_texture) {
                auto diffuse_map_path = material->diffuse_map;
                if (material->diffuse_map.is_relative()) {
                    diffuse_map_path = (scene_dir / material->diffuse_map).string();
                }
                auto diffuse_texture = TextureManager::GetInstance()->CreateTexture(diffuse_map_path);
                diffuse_tex_handle = diffuse_texture->handle();
            }

            auto ai_mesh = meshInfoGrouped.ai_mesh;
            auto mesh = meshInfoGrouped.mesh_info;
            auto offset = meshInfoGrouped.offset;

            auto model_matrix = mesh->transform;
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
                if (!has_diffuse_texture || ai_tex_coords == nullptr) {
                    tex_coords.emplace_back(0.f, 0.f, -1.f);
                } else {
                    tex_coords.emplace_back(ai_tex_coords[i].x, ai_tex_coords[i].y, 1.f);
                    GL_RENDER_INFO("tex coord: ({}, {}, {}), tex uv: ({}, {})",
                                   ai_position.x, ai_position.y, ai_position.z,
                                   ai_tex_coords[i].x, ai_tex_coords[i].y);
                }
                diffuse_vec.emplace_back(diffuse);
                specular_vec.emplace_back(material->specular);
                ambient_vec.emplace_back(material->ambient);
                diffuse_tex_handles.emplace_back(diffuse_tex_handle);
            }

            // process faces
            for (auto i = 0ul; i < ai_mesh->mNumFaces; i++) {
                auto &&face = ai_mesh->mFaces[i].mIndices;
                indices.emplace_back(uint3{face[0], face[1], face[2]} + offset);
            }
        }

        GL_RENDER_INFO(
                "AABB: min = {}, max = {})",
                to_string(_aabb.min),
                to_string(_aabb.max));

        _triangle_count = indices.size();
        auto vertex_count = _triangle_count * 3;

        // transfer to OpenGL
        glGenVertexArrays(1, &_vertex_array);

        vector<uint32_t> buffers(_buffer_num);
        glGenBuffers(_buffer_num, buffers.data());

        _position_buffer = buffers[0];
        _normal_buffer = buffers[1];
        _diffuse_buffer = buffers[2];
        _specular_buffer = buffers[3];
        _ambient_buffer = buffers[4];
        _tex_coord_buffer = buffers[5];
        _diffuse_tex_handle_buffer = buffers[6];

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

        glBindBuffer(GL_ARRAY_BUFFER, _specular_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(specular_vec, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _ambient_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(ambient_vec, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _tex_coord_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float3), impl::_flatten(tex_coords, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _diffuse_tex_handle_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(GLuint64), impl::_flatten(diffuse_tex_handles, indices).data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 1, GL_SAMPLER_2D, GL_FALSE, sizeof(GLuint64), nullptr);

        glBindVertexArray(0);
    }

    GeometryGroup::~GeometryGroup() noexcept {
        glDeleteVertexArrays(1, &_vertex_array);
        glDeleteBuffers(_buffer_num, &_position_buffer);
    }

    void GeometryGroup::render() const {
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
