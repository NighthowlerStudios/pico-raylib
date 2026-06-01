// These files are not to be included by the library compile script.
// Instead, copy and paste it into a new folder in the optionals folder, and include that with a CMakeLists.txt.

#include "pico_display.h"

Orientation currentOrientation;

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

int GetMonitorWidth()
{
    return 0;
}

int GetMonitorHeight()
{
    return 0;
}

void SetMonitorOrientation(Orientation orientation)
{
    currentOrientation = orientation;
}

Orientation GetMonitorOrientation()
{
    return currentOrientation;
}