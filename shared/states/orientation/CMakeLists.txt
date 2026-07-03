add_library(orientation INTERFACE)

target_sources(orientation INTERFACE
    "orientation.c"
)

target_include_directories(orientation INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# pico-raylib already includes a lot.
# set(pico_libs 
#     pio_button
# )

# target_link_libraries(orientation INTERFACE
#     ${pico_libs}
# )