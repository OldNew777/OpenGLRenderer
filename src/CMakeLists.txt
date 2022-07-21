if (WIN32)
    add_compile_definitions(
            NOMINMAX=1
            _ENABLE_EXTENDED_ALIGNED_STORAGE=1
            _CRT_SECURE_NO_WARNINGS=1
            _UNICODE=1)
    set(CMAKE_SHARED_MODULE_PREFIX "") # for MinGW/Cygwin etc.
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/MP)
    endif ()
endif ()

add_subdirectory(ext)

add_library(opengl-render-include INTERFACE)
target_include_directories(opengl-render-include INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(opengl-render-include INTERFACE c_std_11 cxx_std_20)

add_library(opengl-render INTERFACE)

target_link_libraries(opengl-render INTERFACE
        opengl-render-ext)
add_library(opengl::render ALIAS opengl-render)

add_subdirectory(apps)