add_library(vga INTERFACE)

target_sources(vga INTERFACE
    "vga.c"
)

target_include_directories(vga INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../../pico-extras/src/common/pico_scanvideo/include
    ${CMAKE_CURRENT_LIST_DIR}/../../../pico-extras/src/rp2_common/pico_scanvideo_dpi/include
)

# pico-raylib already includes a lot.
# NOTE: include the proper pico_scanvideo_scanline_color_xxxbpp to choose your colour depth per board.
set(pico_libs 
    pico_unique_id
    pico_multicore

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_dma
    hardware_clocks # Support overclocking.
    hardware_vreg

    orientation
)

target_link_libraries(vga INTERFACE
    ${pico_libs}
)