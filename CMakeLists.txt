cmake_minimum_required(VERSION 3.20)
project(libui-hpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(BuildLibuiNg)

add_library(ui INTERFACE)
target_include_directories(ui INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(ui INTERFACE libui_ng)

option(LIBUI_HPP_BUILD_EXAMPLES "Build libui-hpp example applications" ON)

if(LIBUI_HPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
