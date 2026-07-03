add_library(ray_button INTERFACE)

target_sources(ray_button INTERFACE
    "ray_button.c"
)

target_include_directories(ray_button INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
set(pico_libs 
    pio_button
)

target_link_libraries(ray_button INTERFACE
    ${pico_libs}
)