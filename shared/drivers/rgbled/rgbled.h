#ifndef RGBLED_H
#define RGBLED_H

/*
* A reimplementation of the RGBLED driver library by Pimoroni.
* You can find the CPP version of this driver on https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/rgbled
* MIT License

* Copyright (c) 2021 Pimoroni Ltd, Nighthowler Studios (Colin James Wood)

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <stdint.h>

// Processing state colours on the LED to monitor what the CPU is doing even if Raylib has become unresponsive.
#ifdef USE_RGB_LED_AS_DEBUG
    #define SHOW_LED_WAITING_FOR_USB SetRGBLED(rgb, 255, 255, 0, 15)
    #define SHOW_LED_INITIALIZING_ST7789 SetRGBLED(rgb, 255, 0, 0, 15)
    #define SHOW_LED_NO_FRAME_COMMANDED SetRGBLED(rgb, 0, 255, 0, 15)
    #define SHOW_LED_RLSW_DRAWING SetRGBLED(rgb, 0, 255, 255, 15)
    #define SHOW_LED_LCD_DRAWING SetRGBLED(rgb, 255, 0, 255, 15)
    #define SHOW_NO_LED SetRGBLED(rgb, 0, 0, 0, 0)
#else
    #define SHOW_LED_WAITING_FOR_USB
    #define SHOW_LED_INITIALIZING_ST7789 
    #define SHOW_LED_NO_FRAME_COMMANDED
    #define SHOW_LED_RLSW_DRAWING
    #define SHOW_LED_LCD_DRAWING
    #define SHOW_NO_LED SetRGBLED(rgb, 0, 0, 0, 0)
#endif

typedef struct
{
    uint8_t rPin;
    uint8_t gPin;
    uint8_t bPin;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brightness;
} RGBLED;

RGBLED* InitRGBLED(const uint8_t newRPin, const uint8_t newGPin, const uint8_t newBPin);
void SetRGBLED(RGBLED* rgb, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

#endif