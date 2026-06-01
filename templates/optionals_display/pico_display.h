// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder

#define NUM_BUTTONS_TO_TEST 4

#include "raylib.h"

inline const char* GetMonitorDeviceName() { return "Some Device Name"; }

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

void SetMonitorOrientation(Orientation orientation);
Orientation GetMonitorOrientation();

// Emulate keyboards in here.
void PollInput();
// Setup keyboard emulation via GPIO in here.
void InitInput();
