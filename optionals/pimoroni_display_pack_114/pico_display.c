// Copy of templates/optionals_display

#include "pico_display.h"

#include <stdio.h>
#include "pico/stdlib.h"

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

inline const char* GetMonitorDeviceName(void) { return "Pimoroni Pico Display Pack 2.8\""; }

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

extern void InitST7789(uint16_t width, uint16_t height, uint8_t mosi, uint8_t dc, uint8_t sck, uint8_t pwm, uint8_t cs);
extern void SendBufferST7789(int width, int height, const char* buffer);
extern void CleanupST7789(void);

// And now expose this functionality to Raylib.
void InitDisplay(void)
{
    rgb = InitRGBLED(PICO_DISPLAY_LED_R, PICO_DISPLAY_LED_G, PICO_DISPLAY_LED_B);

#ifdef USE_USB_CONSOLE_OUT
#ifdef USE_RGB_LED_AS_DEBUG
    while (!stdio_usb_connected())
    {
        SHOW_LED_WAITING_FOR_USB;
        sleep_ms(250);
        SHOW_NO_LED;
        sleep_ms(250);
    }
#endif
#endif

    SHOW_LED_INITIALIZING_ST7789;

    printf("[DEVICE] Initializing SPI to the LCD with width %i and height %i...\n", PICO_DISPLAY_WIDTH, PICO_DISPLAY_HEIGHT);

    InitST7789(PICO_DISPLAY_WIDTH, PICO_DISPLAY_HEIGHT, SPI_DEFAULT_MOSI, SPI_DEFAULT_DC, SPI_DEFAULT_SCK, SPI_BG_FRONT_PWM, SPI_BG_FRONT_CS);

    SHOW_LED_NO_FRAME_COMMANDED;
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    // In multicore mode this will pulse extremely quickly.
    SHOW_LED_LCD_DRAWING;

    SendBufferST7789(screenWidth, screenHeight, (const char*)buffer);

    SHOW_LED_RLSW_DRAWING;
}

void CleanupDisplay(void)
{
    SHOW_NO_LED;

    CleanupST7789();
}