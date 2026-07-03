add_library(waveshare_pico_lcd_13 INTERFACE)

target_sources(waveshare_pico_lcd_13 INTERFACE
    "pico_display.c"
)

target_include_directories(waveshare_pico_lcd_13 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm

    st7789
    ray_button
)

target_link_libraries(waveshare_pico_lcd_13 INTERFACE
    ${pico_libs}
)