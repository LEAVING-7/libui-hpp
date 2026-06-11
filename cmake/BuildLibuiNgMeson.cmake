if(NOT CONFIG)
    message(FATAL_ERROR "CONFIG is required")
endif()
if(NOT BUILD_ROOT)
    message(FATAL_ERROR "BUILD_ROOT is required")
endif()
if(NOT LIBUI_NG_SOURCE_DIR)
    message(FATAL_ERROR "LIBUI_NG_SOURCE_DIR is required")
endif()
if(NOT LIBUI_MESON_NATIVE_FILE)
    message(FATAL_ERROR "LIBUI_MESON_NATIVE_FILE is required")
endif()

if(CONFIG STREQUAL "Debug")
    set(MESON_BUILDTYPE "debug")
elseif(CONFIG STREQUAL "MinSizeRel")
    set(MESON_BUILDTYPE "minsize")
elseif(CONFIG STREQUAL "RelWithDebInfo")
    set(MESON_BUILDTYPE "debugoptimized")
else()
    set(MESON_BUILDTYPE "release")
endif()

string(TOLOWER "${CONFIG}" CONFIG_LOWER)
set(BUILD_DIR "${BUILD_ROOT}/${CONFIG_LOWER}")
set(LIB_FILE "${BUILD_DIR}/meson-out/libui.a")

set(MESON_SETUP
    meson setup "${BUILD_DIR}" "${LIBUI_NG_SOURCE_DIR}"
    --native-file=${LIBUI_MESON_NATIVE_FILE}
    --default-library=static
    -Dtests=false
    -Dexamples=false
    --buildtype=${MESON_BUILDTYPE}
)

if(MSVC)
    list(APPEND MESON_SETUP --vsenv)
endif()

execute_process(
    COMMAND ${MESON_SETUP}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    RESULT_VARIABLE _setup_result
)
if(NOT _setup_result EQUAL 0)
    message(FATAL_ERROR "meson setup failed for ${CONFIG} (exit ${_setup_result})")
endif()

execute_process(
    COMMAND meson compile -C "${BUILD_DIR}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    RESULT_VARIABLE _compile_result
)
if(NOT _compile_result EQUAL 0)
    message(FATAL_ERROR "meson compile failed for ${CONFIG} (exit ${_compile_result})")
endif()

if(NOT EXISTS "${LIB_FILE}")
    message(FATAL_ERROR "Expected libui-ng library was not produced: ${LIB_FILE}")
endif()
