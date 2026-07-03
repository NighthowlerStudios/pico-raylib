// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"

// Link other libraries here
#include "ray_button.h"

// define static const pin headers here.

// Emulate buttons like keys on the keyboard.
typedef struct PicoButton {
    KeyboardKey key;  // TODO: Don't link the entire raylib header just for this
    uint8_t buttonPin;
    bool isDown;
} PicoButton;

#endif