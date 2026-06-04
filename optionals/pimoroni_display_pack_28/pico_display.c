// Copy of templates/optionals_display

#include "pico_display.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include <stdio.h>

KeyboardKey picoButtons[NUM_BUTTONS_TO_TEST] = { 0 };
Orientation currentOrientation = DISPLAY_ORIENTATION;

static const int PICO_DISPLAY_WIDTH = 320;
static const int PICO_DISPLAY_HEIGHT = 240;
static const uint8_t PICO_DISPLAY_BUTTON_A = 12;
static const uint8_t PICO_DISPLAY_BUTTON_B = 13;
static const uint8_t PICO_DISPLAY_BUTTON_X = 14;
static const uint8_t PICO_DISPLAY_BUTTON_Y = 15;
static const uint8_t PICO_DISPLAY_LED_R = 26;
static const uint8_t PICO_DISPLAY_LED_G = 27;
static const uint8_t PICO_DISPLAY_LED_B = 28;

int GetHardwareResolutionWidth()
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

int GetHardwareResolutionHeight()
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
typedef struct PicoButton {
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
void SetupButton(const uint8_t buttonPin)
{
    gpio_init(buttonPin);
    gpio_set_dir(buttonPin, GPIO_IN);
    gpio_pull_up(buttonPin);
}

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
        picoButtons[i] = gpio_get(picoButtonTable[i].buttonPin) ? KEY_NULL : picoButtonTable[i].key;
    }
}

// Here's where we dissolved the st7789 driver into.
// Please note that most of this is copy-pasted from Pimoroni-Pico.
// Then it was translated over to C, because C++ had some libraries that are incompatible with C99.
uint8_t maSPI_DEFAULT_DCtl;
uint16_t caset[2] = {0, 0};
uint16_t raset[2] = {0, 0};

typedef enum MASPI_DEFAULT_DCTL {
    ROW_ORDER   = 0b10000000,
    COL_ORDER   = 0b01000000,
    SWAP_XY     = 0b00100000,  // AKA "MV"
    SCAN_ORDER  = 0b00010000,
    RGB_BGR     = 0b00001000,
    HORIZ_ORDER = 0b00000100
} __attribute__ ((__packed__)) MASPI_DEFAULT_DCTL;

typedef enum reg {
    SWRESET   = 0x01,
    TEOFF     = 0x34,
    TEON      = 0x35,
    MASPI_DEFAULT_DCTLREG = 0x36,
    COLMOD    = 0x3A,
    RAMCTRL   = 0xB0,
    GCTRL     = 0xB7,
    VCOMS     = 0xBB,
    LCMCTRL   = 0xC0,
    VDVVRHEN  = 0xC2,
    VRHS      = 0xC3,
    VDVS      = 0xC4,
    FRCTRL2   = 0xC6,
    PWCTRL1   = 0xD0,
    PORCTRL   = 0xB2,
    GMCTRP1   = 0xE0,
    GMCTRN1   = 0xE1,
    INVOFF    = 0x20,
    SLPIN     = 0x10,
    SLPOUT    = 0x11,
    DISPON    = 0x29,
    GAMSET    = 0x26,
    DISPOFF   = 0x28,
    RAMWR     = 0x2C,
    INVON     = 0x21,
    CASET     = 0x2A,
    RASET     = 0x2B,
    PWMFRSEL  = 0xCC
} __attribute__ ((__packed__)) reg;

#include <limits.h>
#include "hardware/spi.h"

static const uint8_t PIN_UNUSED = CHAR_MAX; // Intentionally INT_MAX to avoid overflowing MicroPython's int type

static const uint8_t SPI_DEFAULT_MOSI = 19;
static const uint8_t SPI_DEFAULT_DC = 16;
static const uint8_t SPI_DEFAULT_SCK = 18;

static const uint8_t SPI_BG_FRONT_PWM = 20;
static const uint8_t SPI_BG_FRONT_CS = 17;

// interface pins with our standard defaults where appropriate
static spi_inst_t *spi = spi0;
static uint8_t parallel_sm;
static PIO parallel_pio;
static uint8_t parallel_offset;
static uint8_t st_dma;

// The ST7789 requires 16 ns between SPI rising edges.
// 16 ns = 62,500,000 Hz
// 2350 doesn't support 62,500,000 so use 75,000,000 seems to work.
static const uint32_t SPI_BAUD = 75000000;

void command(uint8_t commandChar, int len, const char* data) {
    gpio_put(SPI_DEFAULT_DC, 0); // command mode

    gpio_put(SPI_BG_FRONT_CS, 0);
    
    spi_write_blocking(spi, &commandChar, 1);

    if (data) {
        gpio_put(SPI_DEFAULT_DC, 1); // data mode
        spi_write_blocking(spi, (const uint8_t*)data, len);
    }

    gpio_put(SPI_BG_FRONT_CS, 1);
}

void commandNoString(uint8_t commandChar) 
{ 
    command(commandChar, 0, NULL); 
}

#include <math.h>

void SetBacklight(uint8_t brightness) {
    // gamma correct the provided 0-255 brightness value onto a
    // 0-65535 range for the pwm counter
    float gamma = 2.8;
    uint16_t value = (uint16_t)(pow((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
    pwm_set_gpio_level(SPI_BG_FRONT_PWM, value);
}

// Processing state colours on the LED to monitor what the CPU is doing even if Raylib has become unresponsive.
#ifdef USE_RGB_LED_AS_DEBUG
#define SHOW_LED_WAITING_FOR_USB SetRGBLED(255, 255, 0, 15)
#define SHOW_LED_INITIALIZING_ST7789 SetRGBLED(255, 0, 0, 15)
#define SHOW_LED_NO_FRAME_COMMANDED SetRGBLED(0, 255, 0, 15)
#define SHOW_LED_RLSW_DRAWING SetRGBLED(0, 255, 255, 15)
#define SHOW_LED_LCD_DRAWING SetRGBLED(255, 0, 255, 15)
#define SHOW_NO_LED SetRGBLED(0, 0, 0, 0)
#else
#define SHOW_LED_WAITING_FOR_USB
#define SHOW_LED_INITIALIZING_ST7789 
#define SHOW_LED_NO_FRAME_COMMANDED
#define SHOW_LED_RLSW_DRAWING
#define SHOW_LED_LCD_DRAWING
#define SHOW_NO_LED
#endif

#ifdef MULTICORE

// We use lazy scheduling in here because the framebuffer is read-only on core 2 anyway.
#include "pico/mutex.h"
#include "pico/multicore.h"

int currentWidth = 320;
int currentHeight = 240;
static mutex_t frameBufferMutex;
uint16_t* currentBuffer = NULL;
// Using this as a flag to indicate whether the Raylib buffer has been swapped.
bool flipCompleted = false;

void Core1FlipBuffer(void)
{
    while(true)
    {
        SHOW_LED_RLSW_DRAWING;
        while (!flipCompleted);
        SHOW_LED_LCD_DRAWING;

        mutex_enter_blocking(&frameBufferMutex);

        // Acknowledge.
        flipCompleted = false;

        // Command handles the parsing of the data.  We can't block it nicely, so we use that boolean.
        //printf("Current Buffer Pointer: %p\n", (void*)currentBuffer);
        command(RAMWR, currentWidth * currentHeight * sizeof(uint16_t), (const char*)currentBuffer);

        mutex_exit(&frameBufferMutex);
    }
}

#endif

// And now expose this functionality to Raylib.
void InitDisplay(void)
{
#ifdef USE_USB_CONSOLE_OUT
    stdio_init_all();

    InitRGBLED();

    while (!stdio_usb_connected())
    {
        SHOW_LED_WAITING_FOR_USB;
        sleep_ms(250);
        SHOW_NO_LED;
        sleep_ms(250);
    }
#endif

    SHOW_LED_INITIALIZING_ST7789;

    printf("[DEVICE] Initializing SPI to the LCD...\n");

    // First construct the pin information.
    // configure spi interface and pins
    spi_init(spi, SPI_BAUD);

    gpio_set_function(SPI_DEFAULT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DEFAULT_MOSI, GPIO_FUNC_SPI);

    printf("[DEVICE] Claiming DMA Channel for LCD SPI.\n");

    st_dma = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(st_dma);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_bswap(&config, false);
    channel_config_set_dreq(&config, spi_get_dreq(spi, true));
    dma_channel_configure(st_dma, &config, &spi_get_hw(spi)->dr, NULL, 0, false);

    gpio_set_function(SPI_DEFAULT_DC, GPIO_FUNC_SIO);
    gpio_set_dir(SPI_DEFAULT_DC, GPIO_OUT);

    gpio_set_function(SPI_BG_FRONT_CS, GPIO_FUNC_SIO);
    gpio_set_dir(SPI_BG_FRONT_CS, GPIO_OUT);

    printf("[DEVICE] Backlight off (PWM init)...\n");

    // if a backlight pin is provided then set it up for
    // pwm control
    if(SPI_BG_FRONT_PWM != PIN_UNUSED) {
        pwm_config cfg = pwm_get_default_config();
        pwm_set_wrap(pwm_gpio_to_slice_num(SPI_BG_FRONT_PWM), 65535);
        pwm_init(pwm_gpio_to_slice_num(SPI_BG_FRONT_PWM), &cfg, true);
        gpio_set_function(SPI_BG_FRONT_PWM, GPIO_FUNC_PWM);
        SetBacklight(0); // Turn backlight off initially to avoid nasty surprises
    }

    // Then the full init

    // Common init.
    printf("[DEVICE] Boot the ST7789\n");

    commandNoString(SWRESET);

    commandNoString(TEON);  // enable frame sync signal if used
    command(COLMOD,    1, "\x05");  // 16 bits per pixel

    const char porctrl_bytes[] = {0x0c, 0x0c, 0x00, 0x33, 0x33};
    command(PORCTRL, 5, &porctrl_bytes);
    command(LCMCTRL, 1, "\x2c");
    command(VDVVRHEN, 1, "\x01");
    command(VRHS, 1, "\x12");
    command(VDVS, 1, "\x20");
    const char pwctrl_bytes[] = {0xa4, 0xa1};
    command(PWCTRL1, 2, &pwctrl_bytes);
    command(FRCTRL2, 1, "\x0f");

    // As noted in https://github.com/pimoroni/pimoroni-pico/issues/1040
    // this is required to avoid a weird light grey banding issue with low brightness green.
    // The banding is not visible without tweaking gamma settings (GMCTRP1 & GMCTRN1) but
    // it makes sense to fix it anyway.
    const char ramctrl_bytes[] = { 0x00, 0xc0 };
    command(RAMCTRL, 2, &ramctrl_bytes);

    // Pico Display 2.8 specific.
    command(GCTRL, 1, "\x35");
    command(VCOMS, 1, "\x1f");
    const char gmctrp1_bytes[] = {0xd0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2d };
    const char gmctrn1_bytes[] = {0xd0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0b, 0x16, 0x14, 0x2f, 0x31 };
    command(GMCTRP1, 14, &gmctrp1_bytes);
    command(GMCTRN1, 14, &gmctrn1_bytes);

    commandNoString(INVON);   // set inversion mode
    commandNoString(SLPOUT);  // leave sleep mode
    commandNoString(DISPON);  // turn display on

    sleep_ms(100);

    // Setup the bus boundaries.
    // Pico Display 2.0
    // TODO: Actually make this configurable.
    currentOrientation = LANDSCAPE;

    int width = (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT) ? 240 : 320;
    int height = (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT) ? 320 : 240;
    
    switch (currentOrientation)
    {
        // OpenGL draws the buffer from the bottom left as origin, instead of top left.  
        // So instead of Top to Bottom, we need Bottom to Top.  But still remain Left to Right in all cases.
        // Then, and only after that logic, rotate and reorder.
        case PORTRAIT:
            maSPI_DEFAULT_DCtl = ROW_ORDER;
            break;
        case INVERTED_LANDSCAPE:
            maSPI_DEFAULT_DCtl = ROW_ORDER | COL_ORDER | SWAP_XY;
            break;
        case INVERTED_PORTRAIT:
            maSPI_DEFAULT_DCtl = ROW_ORDER;
            break;
        default: // LANDSCAPE
            maSPI_DEFAULT_DCtl = SWAP_XY;
            break;
    }

    if (width == 320 && height == 240) {
        caset[0] = 0;
        caset[1] = 319;
        raset[0] = 0;
        raset[1] = 239;
    }

    // Pico Display 2.0 at 90 degree rotation
    if (width == 240 && height == 320) {
        caset[0] = 0;
        caset[1] = 239;
        raset[0] = 0;
        raset[1] = 319;
    }

    // Byte swap the 16bit rows/cols values
    caset[0] = __builtin_bswap16(caset[0]);
    caset[1] = __builtin_bswap16(caset[1]);
    raset[0] = __builtin_bswap16(raset[0]);
    raset[1] = __builtin_bswap16(raset[1]);

    command(CASET,  4, (char *)caset);
    command(RASET,  4, (char *)raset);
    command(MASPI_DEFAULT_DCTLREG, 1, (char *)&maSPI_DEFAULT_DCtl);

    printf("[DEVICE] ST7789 is ready for use.\n");

    if(SPI_BG_FRONT_PWM != PIN_UNUSED) {
        //update(); // Send the new buffer to the display to clear any previous content
        sleep_ms(50); // Wait for the update to apply
        SetBacklight(255); // Turn backlight on now surprises have passed
    }

    SHOW_LED_NO_FRAME_COMMANDED;
    
    // Create the mutex and initialize Core 2.
#ifdef MULTICORE
    printf("[DEVICE] Setting up renderer core on Core 2 (1)...\n");

    mutex_init(&frameBufferMutex);
    multicore_reset_core1();
    multicore_launch_core1(Core1FlipBuffer);
#else
    printf("[DEVICE] [WARNING] LCD SPI is set to use the same core as Raylib.  Expect serious bottlenecks!\n");

    SHOW_LED_RLSW_DRAWING;
#endif

   

    printf("[DEVICE] Device ready.\n");
}

void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight)
{
    // Raylib is not allowed to continue until core 2 is done drawing the current buffer.
#ifdef MULTICORE
    while (frameBufferMutex.owner != LOCK_INVALID_OWNER_ID);
    
    currentBuffer = buffer;
    currentWidth = screenWidth;
    currentHeight = screenHeight;
    // On start, this will force Core 1 to wait until first flip, prevents null pointer.
    flipCompleted = true;
#else
// If this is flashing, that's good!  It means the CPU isn't locked up.
    SHOW_LED_LCD_DRAWING;
    command(RAMWR, screenWidth * screenHeight * sizeof(uint16_t), (const char*)buffer);
    SHOW_LED_RLSW_DRAWING;
#endif


}

void CleanupDisplay(void)
{
    // Don't unallocate dma's until the current frame is done drawing.
#ifdef MULTICORE
    while (frameBufferMutex.owner != LOCK_INVALID_OWNER_ID);
#endif
    if(dma_channel_is_claimed(st_dma)) {
        dma_channel_abort(st_dma);
        dma_channel_unclaim(st_dma);
    }

    // Conserve power.
    SHOW_NO_LED;
    SetBacklight(0);
    commandNoString(DISPOFF);
    commandNoString(SLPIN);
}

// Now some exposure for the RGB LED.
pwm_config pwm_cfg;

void SetRGBLED(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    uint16_t r16 = UINT16_MAX - (brightness * r);
    uint16_t g16 = UINT16_MAX - (brightness * g);
    uint16_t b16 = UINT16_MAX - (brightness * b);
    pwm_set_gpio_level(PICO_DISPLAY_LED_R, r16);
    pwm_set_gpio_level(PICO_DISPLAY_LED_G, g16);
    pwm_set_gpio_level(PICO_DISPLAY_LED_B, b16);
}

void InitRGBLED(void)
{
    pwm_cfg = pwm_get_default_config();
    pwm_config_set_wrap(&pwm_cfg, UINT16_MAX);

    pwm_init(pwm_gpio_to_slice_num(PICO_DISPLAY_LED_R), &pwm_cfg, true);
    gpio_set_function(PICO_DISPLAY_LED_R, GPIO_FUNC_PWM);

    pwm_init(pwm_gpio_to_slice_num(PICO_DISPLAY_LED_G), &pwm_cfg, true);
    gpio_set_function(PICO_DISPLAY_LED_G, GPIO_FUNC_PWM);

    pwm_init(pwm_gpio_to_slice_num(PICO_DISPLAY_LED_B), &pwm_cfg, true);
    gpio_set_function(PICO_DISPLAY_LED_B, GPIO_FUNC_PWM);

    SHOW_NO_LED;
}