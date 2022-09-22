//
// Created by ChenXin on 2022/9/21.
//

#include <base/geometry.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace gl_render {

    Geometry::Geometry(const SceneAllNode::SceneAllInfo &sceneAllInfo) {
        std::vector<float3> positions;
        std::vector<float3> normals;
        std::vector<float3> kd;
        std::vector<float3> tex_coords;
        std::vector<float4> tex_properties;
        std::vector<uint3> indices;

        TexturePacker packer;

        for (auto &&mesh : info.meshes()) {

            auto path = info.folder() + mesh.file_name;

            Assimp::Importer importer;
            auto ai_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FixInfacingNormals | aiProcess_GenSmoothNormals);
            if (ai_scene == nullptr || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || ai_scene->mRootNode == nullptr) {
                throw std::runtime_error{serialize("Failed to load scene from: ", path)};
            }

            _mesh_offsets.emplace_back(positions.size());

            auto offset = static_cast<uint32_t>(_mesh_offsets.back());

            // gather submeshes
            std::vector<aiMesh *> mesh_list;
            std::queue<aiNode *> node_queue;
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

            // process submeshes
            for (auto ai_mesh : mesh_list) {

                // process material
                std::string tex_name;
                glm::vec3 color{1.0f};
                glm::vec2 gloss{0.0f, 0.0f};
                if (!mesh.material_name.empty()) {
                    auto iter = info.materials().find(mesh.material_name);
                    if (iter == info.materials().end()) {
                        throw std::runtime_error{serialize("Reference to undefined material: ", mesh.material_name)};
                    }
                    auto &&material = iter->second;
                    if (material.file_name.empty()) {
                        color = material.color;
                    } else {
                        tex_name = material.file_name;
                    }
                    gloss.x = material.specular;
                    gloss.y = material.roughness;
                } else {
                    auto ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
                    if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
                        aiColor3D ai_color;
                        ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color);
                        color = glm::vec3{ai_color.r, ai_color.g, ai_color.b};
                    } else {
                        aiString ai_path;
                        ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_path);
                        tex_name = mesh.file_name;
                        tex_name.erase(tex_name.find('/') + 1).append(ai_path.C_Str());
                    }
                    float shininess;
                    aiColor3D specular;
                    ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
                    ai_material->Get(AI_MATKEY_SHININESS, shininess);
                    gloss.x = (specular.r + specular.g + specular.b) / 3.0f;
                    gloss.y = std::sqrt(2.0f / (2.0f + shininess));
                }

                auto has_texture = false;
                TexturePacker::ImageBlock block{};
                if (!tex_name.empty()) {
                    block = packer.load(info.folder() + tex_name);
                    has_texture = true;
                }

                auto model_matrix = mesh.transform;
                auto normal_matrix = glm::transpose(glm::inverse(glm::mat3{model_matrix}));

                // process vertices
                for (auto i = 0ul; i < ai_mesh->mNumVertices; i++) {
                    auto ai_position = ai_mesh->mVertices[i];
                    auto ai_normal = ai_mesh->mNormals[i];
                    auto position = glm::vec3{model_matrix * glm::vec4{ai_position.x, ai_position.y, ai_position.z, 1.0f}};
                    auto normal = normal_matrix * glm::vec3{ai_normal.x, ai_normal.y, ai_normal.z};
                    positions.emplace_back(position);
                    normals.emplace_back(normal);
                    auto ai_tex_coords = ai_mesh->mTextureCoords[0];
                    if (!has_texture || ai_tex_coords == nullptr) {
                        tex_coords.emplace_back(0.0f, 0.0f, -1.0f);
                        tex_properties.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
                    } else {
                        tex_coords.emplace_back(ai_tex_coords[i].x, ai_tex_coords[i].y, block.index);
                        tex_properties.emplace_back(block.offset.x, block.offset.y, block.size.x, block.size.y);
                    }
                    kd.emplace_back(color);
                    _aabb.min = glm::min(_aabb.min, position);
                    _aabb.max = glm::max(_aabb.max, position);
                }

                // process faces
                for (auto i = 0ul; i < ai_mesh->mNumFaces; i++) {
                    auto &&face = ai_mesh->mFaces[i].mIndices;
                    indices.emplace_back(glm::uvec3{face[0], face[1], face[2]} + offset);
                }

                offset += ai_mesh->mNumVertices;
            }
            _mesh_sizes.emplace_back(positions.size() - _mesh_offsets.back());
        }

        _texture_count = packer.count();
        _texture_array = packer.create_opengl_texture_array();

        auto aabb_min = glm::min(_aabb.min, _aabb.max);
        auto aabb_max = glm::max(_aabb.min, _aabb.max);
        _aabb.min = aabb_min;
        _aabb.max = aabb_max;

        std::cout << "AABB: "
                  << "min = (" << aabb_min.x << ", " << aabb_min.y << ", " << aabb_min.z << "), "
                  << "max = (" << aabb_max.x << ", " << aabb_max.y << ", " << aabb_max.z << ")" << std::endl;

        _triangle_count = indices.size();
        _vertex_count = positions.size();

        std::cout << "Total vertices: " << positions.size() << std::endl;
        std::cout << "Total triangles: " << _triangle_count << std::endl;

        // transfer to OpenGL
        glGenVertexArrays(1, &_vertex_array);

        uint32_t buffers[6];
        glGenBuffers(6, buffers);

        _position_buffer = buffers[0];
        _normal_buffer = buffers[1];
        _color_buffer = buffers[2];
        _tex_coord_buffer = buffers[3];
        _tex_property_buffer = buffers[4];

        glBindVertexArray(_vertex_array);

        glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * 3ul * sizeof(glm::vec3), _flatten(positions, indices).data(), GL_DYNAMIC_COPY);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * 3ul * sizeof(glm::vec3), _flatten(normals, indices).data(), GL_DYNAMIC_COPY);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _color_buffer);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * 3ul * sizeof(glm::vec3), _flatten(kd, indices).data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _tex_coord_buffer);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * 3ul * sizeof(glm::vec3), _flatten(tex_coords, indices).data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _tex_property_buffer);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * 3ul * sizeof(glm::vec4), _flatten(tex_properties, indices).data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

        glBindVertexArray(0);
    }

    Geometry::~Geometry() {
        glDeleteVertexArrays(1, &_vertex_array);
        glDeleteBuffers(6, &_position_buffer);
        glDeleteTextures(1, &_texture_array);
    }

    void Geometry::render(const Shader &shader) const {
        glBindVertexArray(_vertex_array);
        glActiveTexture(GL_TEXTURE0);
        shader.setInt("textures", 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_array);
        glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
        glBindVertexArray(0);
    }

    void Geometry::shadow(const Shader &shader) const {
        glBindVertexArray(_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
        glBindVertexArray(0);
    }

}
