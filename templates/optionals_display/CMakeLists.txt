add_library(pimoroni_display_pack_28 INTERFACE)

target_sources(pimoroni_display_pack_28 INTERFACE
    "pico_display.c"
)

target_include_directories(pimoroni_display_pack_28 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pimoroni_display_pack_28 INTERFACE
    pico_stdlib
    hardware_spi
    hardware_dma
    hardware_clocks
    hardware_timer
    hardware_gpio
)