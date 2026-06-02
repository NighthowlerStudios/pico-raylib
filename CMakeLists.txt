cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 11)
# We're forced to define these by the Pico SDK.
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
    "Pimoroni_Display_Pack_2.8;Pimoroni_VGA_Demo;Pimoroni_DVI_Breakout" 
    "Which video output is wired up?"
)
enum_option(RAYLIB_BOARD 
    "Pimoroni_Pico_Plus2;Pimoroni_Pico_Plus2_W" 
    "Which RP2350 board is being used?"
)

if (RAYLIB_DISPLAY STREQUAL "Pimoroni_VGA_Demo")
    set(PICO_BOARD "vgaboard")
else()
    if (RAYLIB_BOARD STREQUAL "Pimoroni_Pico_Plus2")
        set(PICO_BOARD "pimoroni_pico_plus2_rp2350")
    endif()
    if (RAYLIB_BOARD STREQUAL "Pimoroni_Pico_Plus2_W")
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

# Automatically use PSRAM thanks to the Pico SDK's wrap functionality.
set(SKIP_PICO_MALLOC 1)
set(SFE_PICO_ALLOC_WRAP 1)

pico_sdk_init()
pico_find_compiler(PICO_COMPILER_LD ${PICO_GCC_TRIPLE}-ld)

# https://github.com/georgik/esp-idf-component-raylib/blob/538523e3fb1d3cb8cda3001a189e129bd8482c36/raylib/CMakeLists.txt#L23
set(raylib_defs
    SFE_PICO_ALLOC_WRAP
    GRAPHICS_API_OPENGL_SOFTWARE
    PLATFORM_CUSTOM
    SW_COLOR_BUFFER_BITS=16
    SW_DEPTH_BUFFER_BITS=16
    SW_FRAMEBUFFER_COLOR_TYPE=R5G6B5
)

# We rip apart the raylib sources to stub some of the defines for now.

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
    list(APPEND raylib_defs
        AUX_AUDIO
    )
endif()

if (SD_CARD)
    list(APPEND raylib_sources
        "raylib/src/rmodels.c"
    )
    list(APPEND raylib_defs
        SD_CARD
    )
endif()

add_library(pico_raylib INTERFACE)

target_sources(pico_raylib INTERFACE
    ${raylib_sources}
    "wrapper/src/rcore_pico.c"
)

add_subdirectory("sparkfun-pico/sparkfun_pico")

target_include_directories(pico_raylib INTERFACE
    "wrapper/include" # Force our stubs to override raylib
    "raylib/src"
    "raylib/src/external"
)

pico_enable_stdio_usb(pico_raylib 1)

set(pico_libraries
    pico_stdlib
    # Hardware accelerate float and double.
    pico_float
    pico_double

    # Replace default allocators with PSRAM ones.
    sparkfun_pico

    hardware_dma
    hardware_interp
    hardware_timer
    hardware_spi
)

# Unfortunately we can't use most third party's drivers as they're usually implemented with pico_graphics.
# pico_graphics allocates its own framebuffer, wasting SRAM.
# So we need to implement our own drivers.

# Take the name of the enum as the directory name in optionals.
string(TOLOWER ${RAYLIB_DISPLAY} raylib_display_lower)
string(GENEX_STRIP ${raylib_display_lower} raylib_display_lower)
# For convenient naming availability, strip some characters out to keep the folder names standardised.
string(REPLACE "." "" raylib_display_lower ${raylib_display_lower})

message(STATUS "Selected display: ${RAYLIB_DISPLAY} (${raylib_display_lower})")

# Only one target display is allowed.
add_subdirectory("optionals/${raylib_display_lower}")
list(APPEND pico_libraries
    "${raylib_display_lower}"
)

target_link_libraries(pico_raylib INTERFACE
    ${pico_libraries}
)

target_compile_definitions(pico_raylib INTERFACE
    ${raylib_defs}
)

target_compile_options(pico_raylib INTERFACE
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