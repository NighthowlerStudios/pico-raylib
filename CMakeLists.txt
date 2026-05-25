cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Git REQUIRED)

include(CMakeDependentOption)
include(raylib/cmake/EnumOption.cmake)

set(BUILD_SHARED_LIBS OFF)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico-sdk/CMakeLists.txt")
    message(STATUS "Fetching all submodules, and Pico SDK's submodules...")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${CMAKE_CURRENT_LIST_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND_ERROR_IS_FATAL ANY)
endif()

if (NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
endif()

# For future proofing in case a new rp comes out.
if (NOT DEFINED PICO_PLATFORM)
    set(PICO_PLATFORM "rp2350")
elseif (PICO_PLATFORM STREQUAL "rp2040")
    message(FATAL_ERROR "The rp2040 is not supported by this library.  \nUndefine PICO_PLATFORM or delete CMake cache.")
endif()

enum_option(RAYLIB_DISPLAY 
    "ST7789;Pimoroni VGA Demo;DVI" 
    "Which video output is wired up?"
)
enum_option(RAYLIB_BOARD 
    "Pimoroni Pico Plus2;Pimoroni Pico Plus2 W" 
    "Which RP2350 board is being used?"
)

if (RAYLIB_DISPLAY STREQUAL "Pimoroni VGA Demo")
    set(PICO_BOARD "vgaboard")
else()
    if (RAYLIB_BOARD STREQUAL "Pimoroni Pico Plus2")
        set(PICO_BOARD "pimoroni_pico_plus2_rp2350")
    endif()
    if (RAYLIB_BOARD STREQUAL "Pimoroni Pico Plus2 W")
        set(PICO_BOARD "pimoroni_pico_plus2_w_rp2350")
    endif()
endif()

# The following are still on if using the VGA Demo board.
# Otherwise, they aren't expected to be here on any other board type.
cmake_dependent_option(
    SD_CARD 
    "An SD card module is wired up." 
    OFF
    [[NOT RAYLIB_DISPLAY STREQUAL "Pimoroni VGA Demo"]]
    ON
)

cmake_dependent_option(
    AUX_AUDIO
    "An audio output is wired up."
    OFF
    [[NOT RAYLIB_DISPLAY STREQUAL "Pimoroni VGA Demo"]]
    ON
)

include(pico-sdk/pico_sdk_init.cmake)

# https://github.com/raysan5/raylib/blob/34b06ca5871d7f6fa39e9e37f7f8f453db9f6cee/CMakeLists.txt#L5
# Avoid excessive expansion of variables in conditionals. In particular, if
# "PLATFORM" is "DRM" then:
#
# if (${PLATFORM} MATCHES "DRM")
#
# may expand e.g to:
#
# if (/usr/lib/aarch64-linux-gnu/libdrm.so MATCHES "DRM")
#
# See https://cmake.org/cmake/help/latest/policy/CMP0054.html
cmake_policy(SET CMP0054 NEW)

project(pico_raylib 
    VERSION 1.0.0 
    LANGUAGES C CXX ASM
)

pico_sdk_init()
pico_find_compiler(PICO_COMPILER_LD ${PICO_GCC_TRIPLE}-ld)

set(raylib_sources
    "raylib/src/rcore.c"
    "raylib/src/rtext.c"
    "raylib/src/rtextures.c"
    "raylib/src/rshapes.c"
)

if (AUX_AUDIO)
    list(APPEND raylib_sources
        "raylib/src/raudio.c"
    )
endif()

if (SD_CARD)
    list(APPEND raylib_sources
        "raylib/src/rmodels.c"
    )
endif()

add_library(pico_raylib
    ${raylib_sources}
    "src/rcore_pico.c"
)

target_include_directories(pico_raylib PUBLIC
    "include" # Force our stubs to override raylib
    "raylib/src"
    "raylib/src/external"
)

# https://github.com/georgik/esp-idf-component-raylib/blob/538523e3fb1d3cb8cda3001a189e129bd8482c36/raylib/CMakeLists.txt#L23
target_compile_definitions(pico_raylib PUBLIC
    GRAPHICS_API_OPENGL_SOFTWARE
    PLATFORM_CUSTOM
    SW_COLOR_BUFFER_BITS=16
    SW_DEPTH_BUFFER_BITS=16
    SW_FRAMEBUFFER_COLOR_TYPE=R5G6B5
)

target_compile_options(pico_raylib PUBLIC
    -I${CMAKE_CURRENT_LIST_DIR}/include          # Custom config.h takes precedence
    -Wno-incompatible-pointer-types              # M3D and software renderer compatibility
    -Wno-uninitialized
    -Wno-nonnull
    -Wno-return-type
    #-Wno-unused-variable
    #-Wno-unused-function
    #-Wno-unused-label                           # Software renderer (rlsw.h) has unused labels on previous commits
    -Wno-stringop-truncation                     # Raylib rtext.c TextReplaceBetween
)