// Copy of templates/optionals_display

#define NUM_BUTTONS_TO_TEST 4

#include <stdint.h>
#include "raylib.h"

typedef enum PicoDisplay28 {
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

typedef enum Orientation {
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
void PollInput(void);
// Setup keyboard emulation via GPIO in here.
void InitInput(void);

// Initialise display drivers.
void InitDisplay(void);
// In R5G6B5 format, transmit the buffer.  Block raylib in here if we're waiting for Core 2 to finish.
void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight);

// Clean up the display driver.
void CleanupDisplay(void);

// Change the brightness of the LCD manually.  It's full by default, so maybe some power could be saved here.
void SetBacklight(uint8_t brightness);
