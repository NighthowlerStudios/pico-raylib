add_library(pimoroni_vga_demo INTERFACE)

target_sources(pimoroni_vga_demo INTERFACE
    "pico_display.c"
)

target_include_directories(pimoroni_vga_demo INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    vga
    pico_scanvideo_dpi
)

target_link_libraries(pimoroni_vga_demo INTERFACE
    ${pico_libs}
)