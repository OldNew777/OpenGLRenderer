cmake_minimum_required(VERSION 3.18...3.21)
cmake_policy(VERSION 3.18)

project(OpenGLRenderer LANGUAGES C CXX VERSION 0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET 12)
set(BUILD_SHARED_LIBS ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
            "/Zc:preprocessor"
            "/wd5105"
            "/wd4819")
endif ()

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only supports 64-bit platforms")
endif ()

if (NOT DEFINED OPENGL_RENDER_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(OPENGL_RENDER_MASTER_PROJECT ON)
    else ()
        set(OPENGL_RENDER_MASTER_PROJECT OFF)
    endif ()
endif ()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    option(OPENGL_RENDER_ENABLE_SCCACHE "Enable sccache to speed up compilation" ON)
    if (OPENGL_RENDER_ENABLE_SCCACHE)
        find_program(SCCACHE_EXE sccache)
        if (SCCACHE_EXE)
            message(STATUS "Build with sccache: ${SCCACHE_EXE}")
            set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_EXE})
            set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_EXE})
            set(CMAKE_OBJC_COMPILER_LAUNCHER ${SCCACHE_EXE})
            set(CMAKE_OBJCXX_COMPILER_LAUNCHER ${SCCACHE_EXE})
        else ()
            message(STATUS "Could not find sccache")
        endif ()
    endif ()
endif ()

option(OPENGL_RENDER_BUILD_TESTS "Build tests for OpenGLRenderer" ${OPENGL_RENDER_MASTER_PROJECT})
option(OPENGL_RENDER_ENABLE_UNITY_BUILD "Enable unity build to speed up compilation" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach (CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG}/bin")
endforeach ()

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

add_subdirectory(src)