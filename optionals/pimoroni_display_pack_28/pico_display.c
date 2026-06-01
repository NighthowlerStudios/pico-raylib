// Copy of templates/optionals_display

#include "pico_display.h"
#include "hardware/gpio.h"

Orientation currentOrientation;
KeyboardKey picoButtons[NUM_BUTTONS_TO_TEST] = { 0 };

inline int GetHardwareResolutionWidth()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return 240;
    }   
    else
    {
        return 320;
    }
}

inline int GetHardwareResolutionHeight()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return 320;
    }
    else
    {
        return 240;
    }
}

// st7789 is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMinimumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

// st7789 is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMaximumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

// Emulate buttons like keys on the keyboard.

typedef struct {
    KeyboardKey key;
    uint8_t buttonPin;
} PicoButton;

// Comparison table.
PicoButton picoButtonTable[NUM_BUTTONS_TO_TEST] = {
    { KEY_A, PICO_DISPLAY_BUTTON_A },
    { KEY_B, PICO_DISPLAY_BUTTON_B },
    { KEY_X, PICO_DISPLAY_BUTTON_X },
    { KEY_ESCAPE, PICO_DISPLAY_BUTTON_Y } // Quit button.
};

// Internal
inline void SetupButton(const uint8_t buttonPin)
{
    gpio_init(buttonPin);
    gpio_set_dir(buttonPin, GPIO_IN);
    gpio_pull_down(buttonPin);
}

void InitInput()
{
    SetupButton(PICO_DISPLAY_BUTTON_A);
    SetupButton(PICO_DISPLAY_BUTTON_B);
    SetupButton(PICO_DISPLAY_BUTTON_X);
    SetupButton(PICO_DISPLAY_BUTTON_Y);
}

void PollInput()
{
    for (int i = 0; i < NUM_BUTTONS_TO_TEST; i++)
    {
        picoButtons[i] = gpio_get(picoButtonTable[i].buttonPin) ? picoButtonTable[i].key : KEY_NULL;
    }
}