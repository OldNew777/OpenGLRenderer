add_library(opengl-render-ext INTERFACE)

add_subdirectory(cxxopts)
target_link_libraries(opengl-render-ext INTERFACE cxxopts::cxxopts)