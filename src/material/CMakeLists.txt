set(OPENGL_RENDER_MATERIAL_SOURCES
        phong.cpp)

add_library(opengl-render-material SHARED ${OPENGL_RENDER_MATERIAL_SOURCES})
target_link_libraries(opengl-render-material PUBLIC
        opengl-render-core
        opengl-render-base
        opengl-render-util)
set_target_properties(opengl-render-material PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
