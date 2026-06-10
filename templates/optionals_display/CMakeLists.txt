add_library(optionals_display INTERFACE)

target_sources(optionals_display INTERFACE
    "pico_display.c"
)

target_include_directories(optionals_display INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm
)

target_link_libraries(optionals_display INTERFACE
    ${pico_libs}
)