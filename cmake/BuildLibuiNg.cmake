include(FetchContent)

FetchContent_Declare(
    libui_ng
    GIT_REPOSITORY https://github.com/libui-ng/libui-ng.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(libui_ng)

set(LIBUI_NG_SOURCE_DIR "${libui_ng_SOURCE_DIR}")

set(LIBUI_COMPILER_FINGERPRINT
    "${CMAKE_C_COMPILER}|${CMAKE_CXX_COMPILER}|${CMAKE_BUILD_TYPE}|${CMAKE_GENERATOR}"
)
string(SHA256 LIBUI_COMPILER_HASH "${LIBUI_COMPILER_FINGERPRINT}")
string(SUBSTRING "${LIBUI_COMPILER_HASH}" 0 12 LIBUI_COMPILER_HASH_SHORT)
set(LIBUI_NG_BUILD_ROOT "${CMAKE_BINARY_DIR}/libui-ng-build-${LIBUI_COMPILER_HASH_SHORT}")

# Meson native file: force the same toolchain CMake selected.
string(REPLACE "\\" "/" LIBUI_MESON_C_COMPILER "${CMAKE_C_COMPILER}")
string(REPLACE "\\" "/" LIBUI_MESON_CXX_COMPILER "${CMAKE_CXX_COMPILER}")

set(LIBUI_MESON_NATIVE_FILE "${CMAKE_BINARY_DIR}/libui-ng-native.ini")
set(LIBUI_MESON_NATIVE_LINES "[binaries]\n")
string(APPEND LIBUI_MESON_NATIVE_LINES "c = '${LIBUI_MESON_C_COMPILER}'\n")
string(APPEND LIBUI_MESON_NATIVE_LINES "cpp = '${LIBUI_MESON_CXX_COMPILER}'\n")

if(APPLE)
    string(APPEND LIBUI_MESON_NATIVE_LINES "objc = '${LIBUI_MESON_C_COMPILER}'\n")
    string(APPEND LIBUI_MESON_NATIVE_LINES "objcpp = '${LIBUI_MESON_CXX_COMPILER}'\n")
endif()

if(CMAKE_AR)
    string(REPLACE "\\" "/" LIBUI_MESON_AR "${CMAKE_AR}")
    string(APPEND LIBUI_MESON_NATIVE_LINES "ar = '${LIBUI_MESON_AR}'\n")
endif()

if(CMAKE_RANLIB)
    string(REPLACE "\\" "/" LIBUI_MESON_RANLIB "${CMAKE_RANLIB}")
    string(APPEND LIBUI_MESON_NATIVE_LINES "ranlib = '${LIBUI_MESON_RANLIB}'\n")
endif()

if(WIN32 AND CMAKE_RC_COMPILER)
    string(REPLACE "\\" "/" LIBUI_MESON_RC_COMPILER "${CMAKE_RC_COMPILER}")
    string(APPEND LIBUI_MESON_NATIVE_LINES "windres = '${LIBUI_MESON_RC_COMPILER}'\n")
endif()

file(WRITE "${LIBUI_MESON_NATIVE_FILE}" "${LIBUI_MESON_NATIVE_LINES}")

set(LIBUI_NG_LIB_FILE "${LIBUI_NG_BUILD_ROOT}/$<LOWER_CASE:$<CONFIG>>/meson-out/libui.a")

add_custom_command(
    OUTPUT "${LIBUI_NG_LIB_FILE}"
    COMMAND ${CMAKE_COMMAND}
        -DCONFIG=$<CONFIG>
        -DBUILD_ROOT=${LIBUI_NG_BUILD_ROOT}
        -DLIBUI_NG_SOURCE_DIR=${LIBUI_NG_SOURCE_DIR}
        -DLIBUI_MESON_NATIVE_FILE=${LIBUI_MESON_NATIVE_FILE}
        -DMSVC=$<BOOL:${MSVC}>
        -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_CURRENT_LIST_DIR}/BuildLibuiNgMeson.cmake
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS "${LIBUI_MESON_NATIVE_FILE}"
    COMMENT "Building libui-ng ($<CONFIG>) with Meson using ${CMAKE_CXX_COMPILER}"
    VERBATIM
    COMMAND_EXPAND_LISTS
)

add_custom_target(libui_ng_build DEPENDS "${LIBUI_NG_LIB_FILE}")

add_library(libui_ng STATIC IMPORTED GLOBAL)
add_dependencies(libui_ng libui_ng_build)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${CONFIG}" LIBUI_CONFIG_UPPER)
        string(TOLOWER "${CONFIG}" LIBUI_CONFIG_LOWER)
        set_property(TARGET libui_ng APPEND PROPERTY IMPORTED_CONFIGURATIONS "${CONFIG}")
        set_target_properties(libui_ng PROPERTIES
            "IMPORTED_LOCATION_${LIBUI_CONFIG_UPPER}"
                "${LIBUI_NG_BUILD_ROOT}/${LIBUI_CONFIG_LOWER}/meson-out/libui.a"
        )
    endforeach()
else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" LIBUI_CONFIG_LOWER)
    set_target_properties(libui_ng PROPERTIES
        IMPORTED_LOCATION "${LIBUI_NG_BUILD_ROOT}/${LIBUI_CONFIG_LOWER}/meson-out/libui.a"
    )
endif()

set_target_properties(libui_ng PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LIBUI_NG_SOURCE_DIR}"
    INTERFACE_COMPILE_DEFINITIONS "_UI_STATIC"
)

if(WIN32)
    set(LIBUI_PLATFORM_LIBS
        user32 kernel32 gdi32 comctl32 uxtheme msimg32
        comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid windowscodecs
    )
elseif(APPLE)
    find_library(FW_FOUNDATION  Foundation  REQUIRED)
    find_library(FW_APPKIT      AppKit      REQUIRED)
    find_library(FW_OBJC         objc        REQUIRED)
    set(LIBUI_PLATFORM_LIBS ${FW_FOUNDATION} ${FW_APPKIT} ${FW_OBJC})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
    set(LIBUI_PLATFORM_LIBS PkgConfig::GTK3 m dl)
endif()

target_link_libraries(libui_ng INTERFACE ${LIBUI_PLATFORM_LIBS})
