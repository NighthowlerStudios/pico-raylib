// Copy of templates/optionals_display

#include "pico_display.h"

#include <stdio.h>
#include "pico/stdlib.h"

inline const char* GetMonitorDeviceName(void) { return "Pimoroni Pico Display Pack 1.14\""; }

// Comparison table.
PicoButton picoButtonTable[NUM_BUTTONS_TO_TEST] = {
    { KEY_A, PICO_DISPLAY_BUTTON_A, false },
    { KEY_B, PICO_DISPLAY_BUTTON_B, false },
    { KEY_X, PICO_DISPLAY_BUTTON_X, false },
    { KEY_ESCAPE, PICO_DISPLAY_BUTTON_Y, false } // Quit button.
};

// Our RGB LED.  Implemented with the name the macros expect.
RGBLED* rgb = NULL;

int GetHardwareResolutionWidth()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return PICO_DISPLAY_HEIGHT;
    }   
    else
    {
        return PICO_DISPLAY_WIDTH;
    }
}

int GetHardwareResolutionHeight()
{
    if (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT)
    {
        return PICO_DISPLAY_WIDTH;
    }
    else
    {
        return PICO_DISPLAY_HEIGHT;
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

// Internal use to prevent misuse by the Raylib user.

extern void SetupButton(const uint8_t buttonPin);
extern bool IsButtonDown(const uint8_t buttonPin);

void InitInput(void)
{
    SetupButton(PICO_DISPLAY_BUTTON_A);
    SetupButton(PICO_DISPLAY_BUTTON_B);
    SetupButton(PICO_DISPLAY_BUTTON_X);
    SetupButton(PICO_DISPLAY_BUTTON_Y);
}

void PollInput(void)
{
    for (int i = 0; i < NUM_BUTTONS_TO_TEST; i++)
    {
        // These buttons go low when pressed, not high.
        picoButtonTable[i].isDown = IsButtonDown(picoButtonTable[i].buttonPin);
    }
}

// Internal linkage of these methods to prevent misuse by the Raylib user.

extern void InitST7789(uint16_t width, uint16_t height, uint8_t mosi, uint8_t dc, uint8_t sck, uint8_t pwm, uint8_t cs, bool circular);
extern void SendBufferST7789(int width, int height, const char* buffer);
extern void CleanupST7789(void);

// And now expose this functionality to Raylib.
void InitDisplay(unsigned int width, unsigned int height)
{
    SHOW_LED_INITIALIZING_ST7789;

    printf("[DEVICE] Initializing SPI to the LCD with width %i and height %i...\n", width, height, false);

    InitST7789(width, height, SPI_DEFAULT_MOSI, SPI_DEFAULT_DC, SPI_DEFAULT_SCK, SPI_BG_FRONT_PWM, SPI_BG_FRONT_CS);

    SHOW_LED_NO_FRAME_COMMANDED;
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    // In multicore mode this will pulse extremely quickly.
#ifndef MULTICORE
    SHOW_LED_LCD_DRAWING;
#endif

    SendBufferST7789(screenWidth, screenHeight, (const char*)buffer);

    SHOW_LED_RLSW_DRAWING;
}

void CleanupDisplay(void)
{
    SHOW_NO_LED;

    CleanupST7789();
}