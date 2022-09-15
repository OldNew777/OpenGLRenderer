function(opengl_render_add_application name)
    cmake_parse_arguments(APP "" "" "SOURCES" ${ARGN})
    add_executable(${name} ${APP_SOURCES})
    target_link_libraries(${name} PRIVATE opengl::render)
endfunction()

opengl_render_add_application(opengl-render-cli SOURCES cli.cpp)
