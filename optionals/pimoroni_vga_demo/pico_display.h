// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "vga.h"

// Expose these controls to the including library.


// VGA signals are only stable at certain resolutions.
// For us, this is the most helpful as SRAM will fit.
static const int PICO_DISPLAY_WIDTH = 320;
static const int PICO_DISPLAY_HEIGHT = 240;

#endif