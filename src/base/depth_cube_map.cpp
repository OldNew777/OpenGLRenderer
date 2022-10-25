//
// Created by ChenXin on 2022/10/25.
//

#include "depth_cube_map.h"

namespace gl_render {

    GLuint DepthCubeMap::VERTEX_ARRAY = 0u;
    GLuint DepthCubeMap::POSITION_BUFFER = 0u;
    int DepthCubeMap::TRIANGLE_COUNT = 0u;
    gl_render::unique_ptr<Shader> DepthCubeMap::SHADER = nullptr;
    int DepthCubeMap::INSTANCE_NUM = 0;

}