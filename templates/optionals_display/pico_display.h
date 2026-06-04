// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#define NUM_BUTTONS_TO_TEST 4

#include <stdint.h>
#include "raylib.h"

inline const char* GetMonitorDeviceName() { return "Some Device Name"; }

typedef enum Orientation {
    PORTRAIT = 0,
    LANDSCAPE = 1,
    INVERTED_PORTRAIT = 2,
    INVERTED_LANDSCAPE = 3
} Orientation;

extern Orientation currentOrientation;

// Emulate buttons like keys on the keyboard.
typedef struct PicoButton {
    KeyboardKey key;
    uint8_t buttonPin;
    bool isDown;
} PicoButton;

extern PicoButton picoButtonTable[NUM_BUTTONS_TO_TEST];

// Comparison table.
PicoButton picoButtonTable[NUM_BUTTONS_TO_TEST] = {
    { KEY_A, 0, false },
    { KEY_B, 0, false },
    { KEY_X, 0, false },
    { KEY_ESCAPE, 0, false } // Quit button.
};

// Overrides the minimum resolution with a capper, if needed.
void GetMinimumResolution(int* width, int* height);
// Overrides the maximum resolution with a capper, if needed.
void GetMaximumResolution(int* width, int* height);

// Emulate keyboards in here.
void PollInput(void);
// Setup keyboard emulation via GPIO in here.
void InitInput(void);

// Initialise display drivers.
void InitDisplay(void);
// In R5G6B5 format, transmit the buffer.  Block raylib in here if we're waiting for Core 2 to finish.
void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight);

// Clean up the display driver.
void CleanupDisplay(void);

#endif