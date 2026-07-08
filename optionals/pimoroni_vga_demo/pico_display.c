// Copy of templates/optionals_display

#include "pico_display.h"

#include <stdio.h>
#include "pico/stdlib.h"

inline const char* GetMonitorDeviceName(void) { return "Pimoroni VGA Demo\""; }

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

// vga is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMinimumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

// vga is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMaximumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

void InitInput(void)
{
    // VGA demo doesn't have any buttons.
}

void PollInput(void)
{
    // VGA Demo doesn't have any buttons.
}

// Prevent misuse by Raylib user.
extern void InitVGA(unsigned int width, unsigned int height);
extern void VGAStartCore1(void);
extern void FlipVGAFrameBuffer(uint16_t* framebuffer);
extern void CleanupVGA(void);

// And now expose this functionality to Raylib.
void InitDisplay(unsigned int width, unsigned int height)
{
    printf("[DEVICE] Initializing VGA with width %i and height %i...\n", width, height);

    InitVGA(width, height);
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    FlipVGAFrameBuffer(buffer);
}

void CleanupDisplay(void)
{
    CleanupVGA();
}