set(OPENGL_RENDER_BASE_SOURCES
        camera.h camera.cpp
        depth_cube_map.h depth_cube_map.cpp
        geometry.h geometry.cpp
        hdr2ldr.h
        light.h
        light_manager.h
        pipeline.h pipeline.cpp
        pixel.h
        scene_info.h scene_info.cpp
        scene_parser.h
        shader.h
        texture.h
        texture_manager.h)

add_library(opengl-render-base SHARED ${OPENGL_RENDER_BASE_SOURCES})
target_link_libraries(opengl-render-base PUBLIC
        opengl-render-core
        opengl-render-util)
set_target_properties(opengl-render-base PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
