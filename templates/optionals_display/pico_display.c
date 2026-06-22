// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder, and include that with a CMakeLists.txt.

#include "pico_display.h"

// Comparison table.
PicoButton picoButtonTable[NUM_BUTTONS_TO_TEST] = {
    { KEY_A, PICO_DISPLAY_BUTTON_A, false },
    { KEY_B, PICO_DISPLAY_BUTTON_B, false },
    { KEY_X, PICO_DISPLAY_BUTTON_X, false },
    { KEY_ESCAPE, PICO_DISPLAY_BUTTON_Y, false } // Quit button.
};


void GetMinimumResolution(int* width, int* height)
{
    *width = 320;
    *height = 240;
}

void GetMaximumResolution(int* width, int* height)
{
    *width = 320;
    *height = 240;
}

void InitInput(void)
{

}

void PollInput(void)
{

}

void InitDisplay(void)
{

}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{

}

void CleanupDisplay(void)
{

}