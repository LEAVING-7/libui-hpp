cmake_minimum_required(VERSION 3.20)
project(libui-widget-gallery LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(BuildLibuiNg)

set(GALLERY_SOURCES
    src/main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND GALLERY_SOURCES windows/resources.rc)
endif()

if(WIN32)
    add_executable(widget-gallery WIN32 ${GALLERY_SOURCES})
else()
    add_executable(widget-gallery ${GALLERY_SOURCES})
endif()

target_include_directories(widget-gallery PRIVATE include src)

target_link_libraries(widget-gallery PRIVATE libui_ng)

if(WIN32 AND MSVC)
    # resources.rc already embeds Common Controls 6 manifest; disable linker default.
    # WIN32 sets /SUBSYSTEM:WINDOWS; /ENTRY:mainCRTStartup keeps int main().
    target_link_options(widget-gallery PRIVATE /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup /MANIFEST:NO)
endif()

if(APPLE)
    target_compile_options(widget-gallery PRIVATE -stdlib=libc++)
    target_link_options(widget-gallery PRIVATE -stdlib=libc++)
endif()

add_dependencies(widget-gallery libui_ng_build)

if(WIN32)
    add_executable(table-demo WIN32 examples/table_demo.cpp)
else()
    add_executable(table-demo examples/table_demo.cpp)
endif()

target_include_directories(table-demo PRIVATE include)

target_link_libraries(table-demo PRIVATE libui_ng)

if(WIN32)
    if(MSVC)
        target_sources(table-demo PRIVATE windows/resources.rc)
        target_link_options(table-demo PRIVATE /ENTRY:mainCRTStartup /MANIFEST:NO)
    endif()
endif()

if(APPLE)
    target_compile_options(table-demo PRIVATE -stdlib=libc++)
    target_link_options(table-demo PRIVATE -stdlib=libc++)
endif()

add_dependencies(table-demo libui_ng_build)
