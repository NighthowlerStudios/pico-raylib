#include "pio_button.h"

#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"

// NOTE: Only supports pull up GPIO.

// buttonPin should arrive as a const so you remember which one.
void SetupButton(const uint8_t buttonPin)
{
    gpio_init(buttonPin);
    gpio_set_dir(buttonPin, GPIO_IN);
    gpio_pull_up(buttonPin);
}

// buttonPin should arrive as a const so you remember which one.
// Returns true if down.
bool IsButtonDown(const uint8_t buttonPin)
{
    return !gpio_get(buttonPin);
}