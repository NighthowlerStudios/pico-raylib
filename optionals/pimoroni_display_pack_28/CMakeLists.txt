add_library(pimoroni_display_pack_28 INTERFACE)

target_sources(pimoroni_display_pack_28 INTERFACE
    "pico_display.c"
)

target_include_directories(pimoroni_display_pack_28 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

pico_enable_stdio_usb(pimoroni_display_pack_28 1)
pico_enable_stdio_uart(pimoroni_display_pack_28 0)

set(pico_libs 
    pico_stdlib
    pico_float
    pico_double
    pico_unique_id
    pico_stdio_usb

    hardware_spi
    hardware_dma
    hardware_gpio
    hardware_pio
    hardware_pwm
    hardware_clocks
)

if (NOT SINGLE_CORE) 
    list(APPEND pico_libs 
        picomulticore
    )

    target_compile_definitions(pimoroni_display_pack_28 INTERFACE
        MULTICORE
    )
endif()

target_link_libraries(pimoroni_display_pack_28 INTERFACE
    ${pico_libs}
)