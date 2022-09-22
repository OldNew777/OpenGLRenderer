# backend
add_library(opengl-render-ext SHARED dummy.cpp)
set_target_properties(opengl-render-ext PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(FMT_OS OFF CACHE BOOL "" FORCE)
add_subdirectory(fmt)
target_compile_definitions(fmt-header-only INTERFACE
        FMT_CONSTEVAL=constexpr
        FMT_EXCEPTIONS=0
        FMT_HEADER_ONLY=1
        FMT_USE_NOEXCEPT=1)
target_link_libraries(opengl-render-ext PUBLIC fmt::fmt-header-only)

set(SPDLOG_ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL_HO ON CACHE BOOL "" FORCE)
set(SPDLOG_DISABLE_DEFAULT_LOGGER ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog)
set_target_properties(spdlog PROPERTIES
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
target_link_libraries(spdlog PUBLIC fmt::fmt-header-only)
target_link_libraries(opengl-render-ext PUBLIC spdlog)

add_subdirectory(stb)
target_link_libraries(opengl-render-ext PUBLIC stb)

add_library(xxhash INTERFACE)
target_include_directories(xxhash INTERFACE xxHash)
target_compile_definitions(xxhash INTERFACE XXH_INLINE_ALL)
set_target_properties(xxhash PROPERTIES
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
target_link_libraries(opengl-render-ext PUBLIC xxhash)

set(JSON_MultipleHeaders ON CACHE BOOL "" FORCE)
set(JSON_SystemInclude ON CACHE BOOL "" FORCE)
add_subdirectory(json)
target_link_libraries(opengl-render-ext PUBLIC nlohmann_json::nlohmann_json)

set(glm_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)
target_link_libraries(opengl-render-ext PUBLIC glm)

#add_subdirectory(imgui)
#target_link_libraries(opengl-render-ext PUBLIC imgui)

if (OPENGL_RENDER_ENABLE_GUI)
    add_subdirectory(glad)
    target_link_libraries(opengl-render-ext PUBLIC glad)

    find_package(GLFW3 CONFIG QUIET)
    if (NOT GLFW3_FOUND)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
        add_subdirectory(glfw)
        set_target_properties(glfw PROPERTIES
                UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})
    endif ()
    target_link_libraries(opengl-render-ext PUBLIC glfw)

#    add_subdirectory(imgui)
#    target_link_libraries(opengl-render-ext PUBLIC imgui)
endif ()


# renderer
#add_library(opengl-render-ext INTERFACE)

add_subdirectory(cxxopts)
target_link_libraries(opengl-render-ext INTERFACE cxxopts::cxxopts)

add_library(fast_float INTERFACE)
target_include_directories(fast_float INTERFACE fast_float/include)
target_link_libraries(opengl-render-ext INTERFACE fast_float)

add_library(miniz SHARED tinyexr/deps/miniz/miniz.c)
target_include_directories(miniz PUBLIC tinyexr/deps/miniz)
set_target_properties(miniz PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(tinyexr SHARED tinyexr/tinyexr.cc)
target_include_directories(tinyexr PUBLIC tinyexr)
target_link_libraries(tinyexr PUBLIC miniz)
set_target_properties(tinyexr PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(opengl-render-ext INTERFACE tinyexr)

find_package(assimp CONFIG)
if (NOT assimp_FOUND)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
    set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_PLY_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
    add_subdirectory(assimp)
    set_target_properties(assimp PROPERTIES UNITY_BUILD OFF)
endif ()
target_link_libraries(opengl-render-ext INTERFACE assimp::assimp)