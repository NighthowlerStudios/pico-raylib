add_library(pio_button INTERFACE)

target_sources(pio_button INTERFACE
    "pio_button.c"
)

target_include_directories(pio_button INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    hardware_gpio
    hardware_pio
)

target_link_libraries(pio_button INTERFACE
    ${pico_libs}
)