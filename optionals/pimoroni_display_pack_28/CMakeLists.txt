add_library(pimoroni_display_pack_28 INTERFACE)

target_sources(pimoroni_display_pack_28 INTERFACE
    "pico_display.c"
)

target_include_directories(pimoroni_display_pack_28 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm

    st7789
    rgbled
    pio_button
)

target_link_libraries(pimoroni_display_pack_28 INTERFACE
    ${pico_libs}
)