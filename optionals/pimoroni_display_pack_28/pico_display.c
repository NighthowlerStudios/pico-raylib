// Copy of templates/optionals_display

#include "pico_display.h"

Orientation currentOrientation;

// st7789 is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMinimumResolution(int* width, int* height)
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        *width = 240;
        *height = 320;
    }
    else
    {
        *width = 320;
        *height = 240;
    }
}

// st7789 is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMaximumResolution(int* width, int* height)
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        *width = 240;
        *height = 320;
    }
    else
    {
        *width = 320;
        *height = 240;
    }
}

int GetMonitorWidth()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return 240;
    }   
    else
    {
        return 320;
    }
}

int GetMonitorHeight()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return 320;
    }
    else
    {
        return 240;
    }
}

void SetMonitorOrientation(Orientation orientation)
{
    currentOrientation = orientation;
}

Orientation GetMonitorOrientation()
{
    return currentOrientation;
}