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

#include <stdbool.h>

#include "rgbled.h"

// Now some exposure for the RGB LED.
#include "hardware/pwm.h"
#include "hardware/gpio.h"

pwm_config pwm_cfg;

void SetRGBLED(RGBLED* rgb, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    // Save this data for later reading.
    rgb->r = r;
    rgb->g = g;
    rgb->b = b;
    rgb->brightness = brightness;

    uint16_t r16 = UINT16_MAX - (brightness * r);
    uint16_t g16 = UINT16_MAX - (brightness * g);
    uint16_t b16 = UINT16_MAX - (brightness * b);

    pwm_set_gpio_level(rgb->rPin, r16);
    pwm_set_gpio_level(rgb->gPin, g16);
    pwm_set_gpio_level(rgb->bPin, b16);
}

#include <malloc.h>

RGBLED* InitRGBLED(const uint8_t newRPin, const uint8_t newGPin, const uint8_t newBPin)
{
    pwm_cfg = pwm_get_default_config();
    pwm_config_set_wrap(&pwm_cfg, UINT16_MAX);

    pwm_init(pwm_gpio_to_slice_num(newRPin), &pwm_cfg, true);
    gpio_set_function(newRPin, GPIO_FUNC_PWM);

    pwm_init(pwm_gpio_to_slice_num(newGPin), &pwm_cfg, true);
    gpio_set_function(newGPin, GPIO_FUNC_PWM);

    pwm_init(pwm_gpio_to_slice_num(newBPin), &pwm_cfg, true);
    gpio_set_function(newBPin, GPIO_FUNC_PWM);

    RGBLED* rgb = (RGBLED*)malloc(sizeof(RGBLED));
    rgb->rPin = newRPin;
    rgb->gPin = newGPin;
    rgb->bPin = newBPin;

    SHOW_NO_LED;

    return rgb;
}