add_library(opengl-render-util SHARED
        imageio.cpp imageio.h)
target_link_libraries(opengl-render-util PUBLIC
        opengl-render-include
        opengl-render-ext
        opengl-render-core)
set_target_properties(opengl-render-util PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
