// Copy of templates/optionals_display

#include "pico_display.h"

#include <stdio.h>
#include "pico/stdlib.h"

inline const char* GetMonitorDeviceName(void) { return "Adafruit 1.9 320x170 IPS TFT Display\""; }

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
    *width = 96;
    *height = 96;
}

// st7789 is supposed to be SRAM usable, so we don't allow upscaling in here.
void GetMaximumResolution(int* width, int* height)
{
    *width = GetHardwareResolutionWidth();
    *height = GetHardwareResolutionHeight();
}

void InitInput(void)
{
    // No buttons.
}

void PollInput(void)
{
    // NO buttons.
}

// Internal linkage of these methods to prevent misuse by the Raylib user.

extern void InitST7789(uint16_t width, uint16_t height, uint8_t mosi, uint8_t dc, uint8_t sck, uint8_t pwm, uint8_t cs, bool circular);
extern void SendBufferST7789(int width, int height, const uint16_t* buffer);
extern void CleanupST7789(void);
extern void ForceST7789Reset(uint8_t resetPin);

#include "hardware/gpio.h"

// And now expose this functionality to Raylib.
void InitDisplay(unsigned int width, unsigned int height)
{
    spi_baud = SPI_BAUD;

    printf("[DEVICE] Initializing SPI to the LCD with width %i and height %i...\n", width, height);

    ForceST7789Reset(SPI_RST);

    InitST7789(width, height, SPI_DEFAULT_MOSI, SPI_DEFAULT_DC, SPI_DEFAULT_SCK, SPI_BG_FRONT_PWM, SPI_BG_FRONT_CS, false);
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    SendBufferST7789(screenWidth, screenHeight, buffer);
}

extern void WaitForDMA(void);
extern void CommandClearBlack(void);
extern void ResizeWindowST7789(uint16_t width, uint16_t height, bool circular);
extern void swResize(int w, int h);
void ResizeDisplay(int newWidth, int newHeight)
{
    // Don't allow reallocations until the ST7789 is done with the previous DMA transfer.  This is a blocking call.
    WaitForDMA();
    swResize(newWidth, newHeight);

    // Enforces letterboxing.
    CommandClearBlack();
    ResizeWindowST7789(newWidth, newHeight, false);
}

void CleanupDisplay(void)
{
    CleanupST7789();
}