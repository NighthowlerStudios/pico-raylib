#ifndef RAY_BUTTON_H
#define RAY_BUTTON_H

#include "raylib.h"
#include <stdint.h>

#define RAY_BUTTON_IMPLEMENTATION 1

// Emulate buttons like keys on the keyboard.
typedef struct PicoButton {
    KeyboardKey key;  // TODO: Don't link the entire raylib header just for this
    uint8_t buttonPin;
    bool isDown;
} PicoButton;

// Number of buttons - set by each pico_display.c implementation
extern int numButtonsToTest;
extern PicoButton picoButtonTable[];

// If you edit picoButtonTable at any time, reinitialize all the gpio with this.
void SetupAllButtons(void);
void PollAllButtons(void);

#endif