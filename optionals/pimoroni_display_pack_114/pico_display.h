// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

// Keep visible for rcore_pico.c to see the extern dimensions
#define NUM_BUTTONS_TO_TEST 4

#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"

// Expose these controls to the including library.
#include "st7789.h"
#include "rgbled.h"
#include "pio_button.h"

// Emulate buttons like keys on the keyboard.
typedef struct PicoButton {
    KeyboardKey key;  // TODO: Don't link the entire raylib header just for this
    uint8_t buttonPin;
    bool isDown;
} PicoButton;

#endif