set(OPENGL_RENDER_CORE_SOURCES
        logger.cpp logger.h
        stl.h)

find_package(Threads REQUIRED)
add_library(opengl-render-core SHARED ${OPENGL_RENDER_CORE_SOURCES})
target_compile_definitions(opengl-render-core PRIVATE OPENGL_RENDER_CORE_EXPORT_DLL=1)

target_link_libraries(opengl-render-core PUBLIC
        opengl-render-include
        opengl-render-ext
        Threads::Threads
        ${CMAKE_DL_LIBS}
        $<$<PLATFORM_ID:Windows>:dbghelp>)
set_target_properties(opengl-render-core PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        UNITY_BUILD ${OPENGL_RENDER_ENABLE_UNITY_BUILD})

if (APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(opengl-render-core INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
    target_link_options(opengl-render-core INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
endif ()
