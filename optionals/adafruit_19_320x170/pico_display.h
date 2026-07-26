// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>

// Expose these controls to the including library.
#include "st7789.h"

// This board doesn't have stable traces on it.
#define SPI_BAUD 40000000

// No RGB LED debugs
#define SHOW_LED_WAITING_FOR_USB
#define SHOW_LED_INITIALIZING 
#define SHOW_LED_NO_FRAME_COMMANDED
#define SHOW_LED_RLSW_DRAWING
#define SHOW_LED_DISPLAY_DRAWING
#define SHOW_NO_LED 

static const unsigned int PICO_DISPLAY_WIDTH = 320;
static const unsigned int PICO_DISPLAY_HEIGHT = 170;

static const uint8_t SPI_DEFAULT_MOSI = 19;
static const uint8_t SPI_DEFAULT_MISO = 16;
static const uint8_t SPI_DEFAULT_DC = 20;
static const uint8_t SPI_DEFAULT_SCK = 18;

static const uint8_t SPI_BG_FRONT_PWM = 21;
static const uint8_t SPI_BG_FRONT_CS = 17;

static const uint8_t SPI_RST = 26;

static const uint8_t SPI_SD_CS = 22;

#endif