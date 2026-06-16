cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 11)
# If you wish
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For performance and binary size, we use this as default.  Use Debug manually if you want to use a Debug Module (external hardware required)
set(CMAKE_BUILD_TYPE Release)

find_package(Git REQUIRED)

include(CMakeDependentOption)

set(BUILD_SHARED_LIBS OFF)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico-sdk/CMakeLists.txt")
    message(STATUS "Fetching all submodules, and Pico SDK's submodules...")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${CMAKE_CURRENT_LIST_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND_ERROR_IS_FATAL ANY)
endif()

# Raylib has some helpers for us.  
include(raylib/cmake/EnumOption.cmake)

if (NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
endif()

# For future proofing in case a new rp comes out.
if (NOT DEFINED PICO_PLATFORM)
    set(PICO_PLATFORM "rp2350")
elseif (PICO_PLATFORM STREQUAL "rp2040")
    message(FATAL_ERROR "The rp2040 is not supported by this library.  \nUndefine PICO_PLATFORM or delete CMake cache.")
endif()

option(BUILD_RAYLIB_EXAMPLES "Build the examples folder inside of the Raylib submodule using this version of the library." OFF)
option(BUILD_EXAMPLE "Build the main example that demonstrates everything possible." ON)
option(USE_USB_CONSOLE_OUT "Wait until the USB is connected before going forward with InitWindow()" OFF)
option(USE_RGB_LED_AS_DEBUG "Use the RGBLED to show what state the renderer is in." OFF)

enum_option(RAYLIB_DISPLAY 
    "Pimoroni_Display_Pack_2.8;Pimoroni_VGA_Demo;Pimoroni_DVI_Breakout" 
    "Which video output is wired up?"
)
enum_option(RAYLIB_BOARD 
    "Pimoroni_Pico_Plus2;Pimoroni_Pico_Plus2_W" 
    "Which RP2350 board is being used?"
)
enum_option(DISPLAY_ORIENTATION
    "Landscape;Portrait;Inverted_Landscape;Inverted_Portrait"
    "Which orientation is the screen?"
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
    OFF # The below is true, this option is now configurable
    [[NOT RAYLIB_DISPLAY STREQUAL "Pimoroni_VGA_Demo"]]
    ON # The above is false, this option is now locked to ON
)

cmake_dependent_option(
    AUX_AUDIO
    "An audio output is wired up."
    OFF # The below is true, this option is now configurable
    [[NOT RAYLIB_DISPLAY STREQUAL "Pimoroni_VGA_Demo"]]
    ON # The above is false, this option is now locked to ON
)

# The following remain ON because VGA and DVI need a carefully timed core to bit-bang.
cmake_dependent_option(
    MULTICORE
    "Don't use the same core Raylib uses to draw to the LCD.  Required with VGA/DVI"
    OFF # The below is true, this option is now configurable
    [[NOT RAYLIB_DISPLAY STREQUAL "Pimoroni_VGA_Demo" 
    OR NOT RAYLIB_DISPLAY STREQUAL "Pimoroni_DVI_Breakout"]]
    ON # The above is false, this option is now locked to ON
)

include(pico-sdk/pico_sdk_init.cmake)

include(pico-extras/external/pico_extras_import.cmake)

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
    LANGUAGES C ASM
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

if ((RAYLIB_BOARD STREQUAL "Pimoroni_Pico_Plus2") OR (RAYLIB_BOARD STREQUAL "Pimoroni_Pico_Plus2_W"))
        list(APPEND raylib_defs 
            SFE_RP2350_XIP_CSI_PIN=PIMORONI_PICO_PLUS2_PSRAM_CS_PIN
        )
endif()

# TODO
# Utilize the enum found in pico_display.h
# Note, if you copied a new one to optionals, that enum must be defined!  Otherwise we lose this convenience.
if (DISPLAY_ORIENTATION STREQUAL "Landscape")
    list(APPEND raylib_defs 
        DISPLAY_ORIENTATION=0
    )
elseif (DISPLAY_ORIENTATION STREQUAL "Portrait")
    list(APPEND raylib_defs 
        DISPLAY_ORIENTATION=1
    )
elseif (DISPLAY_ORIENTATION STREQUAL "Inverted_Landscape")
    list(APPEND raylib_defs 
        DISPLAY_ORIENTATION=2
    )
elseif(DISPLAY_ORIENTATION STREQUAL "Inverted_Portrait")
    list(APPEND raylib_defs 
        DISPLAY_ORIENTATION=3
    )
endif()

if (USE_RGB_LED_AS_DEBUG)
    list(APPEND raylib_defs 
        USE_RGB_LED_AS_DEBUG
    )
endif()

if (MULTICORE)
    list(APPEND raylib_defs
        MULTICORE
        RLSW_BACKBUFFER
    )
endif()

# We rip apart the raylib sources to stub some of the defines for now.

set(raylib_sources
    "raylib/src/rcore.c"
    "raylib/src/rtext.c"
    "raylib/src/rtextures.c"
    "raylib/src/rshapes.c"
    "raylib/src/rmodels.c"
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
    list(APPEND raylib_defs
        SD_CARD
    )
endif()

if (USE_USB_CONSOLE_OUT)
    list(APPEND raylib_defs
        USE_USB_CONSOLE_OUT
    )
endif()

# Note, this is the recommended way to compile the SDK.
# However due to the sheer size, maybe we can consider compiling the library once as shared in the future.  
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

set(pico_libraries
    pico_stdlib
    # Hardware accelerate float and double.
    pico_float
    pico_double
    pico_rand # Init random seed, for now.

    # Replace default allocators with PSRAM ones.
    sparkfun_pico

    hardware_dma
    hardware_interp
    hardware_timer
    hardware_spi
    hardware_clocks
    hardware_vreg
    hardware_sleep #pico-extras
    hardware_sleep_headers #pico-extras
)

if (MULTICORE)
    list(APPEND pico_libraries
        pico_multicore
        pico_sync
    )
endif()

if (USE_USB_CONSOLE_OUT)
    list(APPEND pico_libraries
        pico_stdio_usb
    )

    pico_enable_stdio_usb(pico_raylib 1)
    pico_enable_stdio_uart(pico_raylib 0)
endif()

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
    -I${CMAKE_CURRENT_LIST_DIR}/wrapper/include          # Custom config.h takes precedence
    -Wno-incompatible-pointer-types              # M3D and software renderer compatibility
    -Wno-uninitialized
    -Wno-nonnull
    -Wno-return-type
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-unused-label                           # Software renderer (rlsw.h) has unused labels on previous commits
    -Wno-stringop-truncation                     # Raylib rtext.c TextReplaceBetween
)

if (BUILD_EXAMPLE)
    project(example)
    
    add_executable(example "example/example.c")

    target_link_libraries(example 
        pico_raylib
    )

    pico_add_extra_outputs(example)
endif()

if (BUILD_RAYLIB_EXAMPLES)
    # Derived from Raylib's CMakeLists.txt in the examples folder.
    # Setup the project and settings
    project(examples)

    # Directories that contain examples
    set(example_dirs
        "raylib/examples/core"
        "raylib/examples/models"
        "raylib/examples/others"
        "raylib/examples/shapes"
        "raylib/examples/text"
        "raylib/examples/textures"
    )

    if (AUX_AUDIO)
        list(APPEND example_dirs "raylib/examples/audio")
    endif()

    include(CheckIncludeFile)

    # Collect all source files and resource files
    # into a CMake variable
    set(example_sources)
    set(example_resources)
    foreach (example_dir ${example_dirs})
        # Get the .c files
        file(GLOB sources ${example_dir}/*.c)
        list(APPEND example_sources ${sources})

        # Any any resources
        file(GLOB resources ${example_dir}/resources/*)
        list(APPEND example_resources ${resources})
    endforeach ()

    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_loading_thread.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/others/raylib_opengl_interop.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/others/rlgl_standalone.c)

    if (NOT SUPPORT_GESTURES_SYSTEM)
        # Items requiring gestures system
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/textures/textures_mouse_painting.c)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/core/core_basic_screen_manager.c)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/core/core_input_gestures_web.c)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/core/core_input_gestures.c)
    endif ()

    # The audio examples fail to link if raylib is built without raudio
    if (NOT AUX_AUDIO)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/others/embedded_files_loading.c)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/textures/textures_sprite_button.c)
        list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/raylib/examples/textures/textures_sprite_explosion.c)
    endif()

    if (NOT SD_CARD)
        # TODO
    endif()

    include_directories(BEFORE SYSTEM "raylib/examples/others/external/include")

    # Do each example
    foreach (example_source ${example_sources})
        # Create the basename for the example
        get_filename_component(example_name ${example_source} NAME)
        string(REPLACE ".c" "" example_name ${example_name})

        # Setup the example
        add_executable(${example_name} ${example_source})

        target_link_libraries(${example_name} pico_raylib)

        # TODO: move these to an SD card instead
        string(REGEX MATCH "raylib/examples/.*/.*/" resources_dir ${example_source})
        string(APPEND resources_dir "raylib/examples/resources")

        pico_add_extra_outputs(${example_name})
    endforeach ()

    # Copy all of the resource files to the destination
    file(COPY ${example_resources} DESTINATION "raylib/examples/resources/")

endif()
