add_library(pimoroni_display_pack_114 INTERFACE)

target_sources(pimoroni_display_pack_114 INTERFACE
    "pico_display.c"
)

target_include_directories(pimoroni_display_pack_114 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm

    st7789
    rgbled
    ray_button
)

target_link_libraries(pimoroni_display_pack_114 INTERFACE
    ${pico_libs}
)