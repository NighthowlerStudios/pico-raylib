add_library(st7789 INTERFACE)

target_sources(st7789 INTERFACE
    "st7789.c"
)

target_include_directories(st7789 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm
)

target_link_libraries(st7789 INTERFACE
    ${pico_libs}
)