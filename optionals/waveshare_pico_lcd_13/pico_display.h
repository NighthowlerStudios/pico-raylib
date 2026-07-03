// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"

// Expose these controls to the including library.
#include "st7789.h"
#include "ray_button.h"

// No RGB LED debugs
#define SHOW_LED_WAITING_FOR_USB
#define SHOW_LED_INITIALIZING 
#define SHOW_LED_NO_FRAME_COMMANDED
#define SHOW_LED_RLSW_DRAWING
#define SHOW_LED_DISPLAY_DRAWING
#define SHOW_NO_LED 

static const int PICO_DISPLAY_WIDTH = 240;
static const int PICO_DISPLAY_HEIGHT = 240;
static const uint8_t PICO_DISPLAY_BUTTON_A = 15;
static const uint8_t PICO_DISPLAY_BUTTON_B = 17;
static const uint8_t PICO_DISPLAY_BUTTON_X = 19;
static const uint8_t PICO_DISPLAY_BUTTON_Y = 21;
static const uint8_t PICO_DISPLAY_JOY_UP = 2;
static const uint8_t PICO_DISPLAY_JOY_DOWN = 18;
static const uint8_t PICO_DISPLAY_JOY_LEFT = 16;
static const uint8_t PICO_DISPLAY_JOY_RIGHT = 20;
static const uint8_t PICO_DISPLAY_JOY_PRESS = 3;

static const uint8_t SPI_DEFAULT_MOSI = 11;
static const uint8_t SPI_DEFAULT_DC = 8;
static const uint8_t SPI_DEFAULT_SCK = 10;

static const uint8_t SPI_BG_FRONT_PWM = 13;
static const uint8_t SPI_BG_FRONT_CS = 9;

static const uint8_t SPI_RST = 12;

#endif