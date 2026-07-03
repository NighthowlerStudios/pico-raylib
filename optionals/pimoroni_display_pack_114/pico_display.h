// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"

// Expose these controls to the including library.
#include "st7789.h"
#include "rgbled.h"
#include "ray_button.h"

static const int PICO_DISPLAY_WIDTH = 240;
static const int PICO_DISPLAY_HEIGHT = 135;
static const uint8_t PICO_DISPLAY_BUTTON_A = 12;
static const uint8_t PICO_DISPLAY_BUTTON_B = 13;
static const uint8_t PICO_DISPLAY_BUTTON_X = 14;
static const uint8_t PICO_DISPLAY_BUTTON_Y = 15;
static const uint8_t PICO_DISPLAY_LED_R = 6;
static const uint8_t PICO_DISPLAY_LED_G = 7;
static const uint8_t PICO_DISPLAY_LED_B = 8;

static const uint8_t SPI_DEFAULT_MOSI = 19;
static const uint8_t SPI_DEFAULT_DC = 16;
static const uint8_t SPI_DEFAULT_SCK = 18;

static const uint8_t SPI_BG_FRONT_PWM = 20;
static const uint8_t SPI_BG_FRONT_CS = 17;

#endif