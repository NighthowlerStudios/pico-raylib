// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder, and include that with a CMakeLists.txt.

#include "pico_display.h"

#include <stdio.h>
#include "pico/stdlib.h"

// Internal linkage of driver methods should go here to prevent misuse by the Raylib user.

inline const char* GetMonitorDeviceName(void) { return "Pimoroni Pico Display Pack 1.14\""; }

// Comparison table.  ONLY USE IF RAY BUTTON INCLUDED, otherwise delete.
PicoButton picoButtonTable[] = {
    { KEY_A, PICO_DISPLAY_BUTTON_A, false },
    { KEY_B, PICO_DISPLAY_BUTTON_B, false },
    { KEY_X, PICO_DISPLAY_BUTTON_X, false },
    { KEY_ESCAPE, PICO_DISPLAY_BUTTON_Y, false } // Quit button.
};
int numButtonsToTest = 4;

int GetHardwareResolutionWidth()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return PICO_DISPLAY_HEIGHT;
    }   
    else
    {
        return PICO_DISPLAY_WIDTH;
    }
}

int GetHardwareResolutionHeight()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return PICO_DISPLAY_WIDTH;
    }
    else
    {
        return PICO_DISPLAY_HEIGHT;
    }
}

void GetMinimumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

void GetMaximumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

void InitInput(void)
{
    SetupAllButtons();
}

// Privated to avoid Raylib user misuse.
extern void PollAllButtons(void);
void PollInput(void)
{
    PollAllButtons();
}

void InitDisplay(unsigned int width, unsigned int height)
{
    SHOW_LED_INITIALIZING;

    SHOW_LED_NO_FRAME_COMMANDED;
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    // In multicore mode this will pulse extremely quickly.
#ifndef MULTICORE
    SHOW_LED_DISPLAY_DRAWING;
#endif

    SHOW_LED_RLSW_DRAWING;
}

void CleanupDisplay(void)
{
    SHOW_NO_LED;

}