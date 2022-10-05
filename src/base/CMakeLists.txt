set(OPENGL_RENDER_BASE_SOURCES
        camera.h camera.cpp
        geometry.h geometry.cpp
        pipeline.h pipeline.cpp
        pixel.h
        scene_info.h
        scene_parser.h scene_parser.cpp
        shader.h
        texture_packer.h texture_packer.cpp)

add_library(opengl-render-base SHARED ${OPENGL_RENDER_BASE_SOURCES})
target_link_libraries(opengl-render-base PUBLIC
        opengl-render-core
        opengl-render-util)
set_target_properties(opengl-render-base PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
