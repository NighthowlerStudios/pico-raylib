// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "vga.h"


// VGA signals are only stable at certain resolutions.
// For us, this is the most helpful as SRAM will fit.
static const int PICO_DISPLAY_WIDTH = 320;
static const int PICO_DISPLAY_HEIGHT = 240;

// VGA pins are set in CMake.

#ifdef AUX_AUDIO
static const int PICO_AUDIO_LEFT = 26;
static const int PICO_AUDIO_RIGHT = 27;
#endif

#ifdef SD_CARD
static const int SD_CARD_MOSI_PIN = 19;
static const int SD_CARD_MISO_PIN = 20;
static const int SD_CARD_CS_PIN = 21;
static const int SD_CARD_SCK_PIN = 22;
#endif

#endif