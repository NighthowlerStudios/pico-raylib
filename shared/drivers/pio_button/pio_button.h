#ifndef PIO_BUTTON_H
#define PIO_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

// This would be on top of whatever pico_display.c is doing.  Be careful!
void SetupButton(const uint8_t buttonPin);

// Returns true if down.
bool IsButtonDown(const uint8_t buttonPin);

#endif