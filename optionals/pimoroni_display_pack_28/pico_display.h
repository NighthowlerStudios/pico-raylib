// Copy of templates/optionals_display

#ifndef PICO_DISPLAY_H
#define PICO_DISPLAY_H

// Keep visible for rcore_pico.c to see the extern dimensions
#define NUM_BUTTONS_TO_TEST 4

#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"

// Expose these controls to the including library.
#include "st7789.h"
#include "rgbled.h"
#include "pio_button.h"

static const int PICO_DISPLAY_WIDTH = 320;
static const int PICO_DISPLAY_HEIGHT = 240;
static const uint8_t PICO_DISPLAY_BUTTON_A = 12;
static const uint8_t PICO_DISPLAY_BUTTON_B = 13;
static const uint8_t PICO_DISPLAY_BUTTON_X = 14;
static const uint8_t PICO_DISPLAY_BUTTON_Y = 15;
static const uint8_t PICO_DISPLAY_LED_R = 26;
static const uint8_t PICO_DISPLAY_LED_G = 27;
static const uint8_t PICO_DISPLAY_LED_B = 28;

static const uint8_t SPI_DEFAULT_MOSI = 19;
static const uint8_t SPI_DEFAULT_DC = 16;
static const uint8_t SPI_DEFAULT_SCK = 18;

static const uint8_t SPI_BG_FRONT_PWM = 20;
static const uint8_t SPI_BG_FRONT_CS = 17;

// Emulate buttons like keys on the keyboard.
typedef struct PicoButton {
    KeyboardKey key;  // TODO: Don't link the entire raylib header just for this
    uint8_t buttonPin;
    bool isDown;
} PicoButton;

#endif