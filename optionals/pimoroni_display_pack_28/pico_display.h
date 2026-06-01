// Copy of templates/optionals_display

#define NUM_BUTTONS_TO_TEST 4

#include <stdint.h>
#include "raylib.h"

typedef enum {
    PICO_DISPLAY_WIDTH = 320,
    PICO_DISPLAY_HEIGHT = 240,
    PICO_DISPLAY_BUTTON_A = 12,
    PICO_DISPLAY_BUTTON_B = 13,
    PICO_DISPLAY_BUTTON_X = 14,
    PICO_DISPLAY_BUTTON_Y = 15,
    PICO_DISPLAY_LED_R = 26,
    PICO_DISPLAY_LED_G = 27,
    PICO_DISPLAY_LED_B = 28
} PicoDisplay28;

inline const char* GetMonitorDeviceName() { return "Pimoroni Pico Display Pack 2.8\""; }

typedef enum {
    PORTRAIT = 0,
    LANDSCAPE = 1,
    INVERTED_PORTRAIT = 2,
    INVERTED_LANDSCAPE = 3
} Orientation;

extern Orientation currentOrientation;

// Used as a buffer for hardware buttons that should emulate keyboard keys.
extern KeyboardKey picoButtons[NUM_BUTTONS_TO_TEST];

// Overrides the minimum resolution with a capper, if needed.
void GetMinimumResolution(int* width, int* height);
// Overrides the maximum resolution with a capper, if needed.
void GetMaximumResolution(int* width, int* height);

// Emulate keyboards in here.
void PollInput();
// Setup keyboard emulation via GPIO in here.
void InitInput();

