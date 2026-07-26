add_library(adafruit_19_320x170 INTERFACE)

target_sources(adafruit_19_320x170 INTERFACE
    "pico_display.c"
)

target_include_directories(adafruit_19_320x170 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pico_unique_id

    hardware_gpio
    hardware_pio #Needed to sync the peri clk
    hardware_pwm

    st7789
)

target_link_libraries(adafruit_19_320x170 INTERFACE
    ${pico_libs}
)