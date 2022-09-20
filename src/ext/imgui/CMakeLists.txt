file(GLOB IMGUI_SOURCE_FILES
        imgui/*.h
        imgui/*.cpp
        imgui/backends/imgui_impl_glfw.h
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.h
        imgui/backends/imgui_impl_opengl3.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(imgui SHARED ${IMGUI_SOURCE_FILES})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)