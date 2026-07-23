#ifndef ST7789_H
#define ST7789_H

/*
* A reimplementation of the ST7789 driver library by Pimoroni.
* You can find the CPP version of this driver on https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/st7789
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

// Drivers should dictate if orientation is supported.  
#include "orientation.h"

// The ST7789 requires 16 ns between SPI rising edges.
// 16 ns = 62,500,000 Hz
// We are write only, so going past 62,500,000 is a safe overclock.
#define SPI_BAUD 62500000 // Can go up to 75000000 on some displays with tight traces.

// Set the backlight manually.
void SetBacklight(uint8_t brightness);

#endif