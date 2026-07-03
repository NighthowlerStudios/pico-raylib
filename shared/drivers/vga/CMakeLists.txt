add_library(vga INTERFACE)

target_sources(vga INTERFACE
    "vga.c"
)

target_include_directories(vga INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_dma
    hardware_clocks # Support overclocking.
    hardware_vreg

    pico_scanvideo_scanline_color_16bpp
)

target_link_libraries(vga INTERFACE
    ${pico_libs}
)