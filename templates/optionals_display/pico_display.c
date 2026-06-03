// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder, and include that with a CMakeLists.txt.

#include "pico_display.h"

KeyboardKey picoButtons[NUM_BUTTONS_TO_TEST] = { 0 };
Orientation currentOrientation = LANDSCAPE;

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