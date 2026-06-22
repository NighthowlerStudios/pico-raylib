add_library(rgbled INTERFACE)

target_sources(rgbled INTERFACE
    "rgbled.c"
)

target_include_directories(rgbled INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    hardware_gpio
    hardware_pwm
)

target_link_libraries(rgbled INTERFACE
    ${pico_libs}
)