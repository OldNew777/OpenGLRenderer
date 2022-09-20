set(OPENGL_RENDER_BASE_SOURCES
        camera.h camera.cpp)

add_library(opengl-render-base SHARED ${OPENGL_RENDER_BASE_SOURCES})
target_link_libraries(opengl-render-base PUBLIC
        opengl-render-core)
set_target_properties(opengl-render-base PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
