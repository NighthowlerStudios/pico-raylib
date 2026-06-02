// Copy of templates/optionals_display

#include "pico_display.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

KeyboardKey picoButtons[NUM_BUTTONS_TO_TEST] = { 0 };
Orientation currentOrientation = LANDSCAPE;

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
inline void SetupButton(const uint8_t buttonPin)
{
    gpio_init(buttonPin);
    gpio_set_dir(buttonPin, GPIO_IN);
    gpio_pull_down(buttonPin);
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
        picoButtons[i] = gpio_get(picoButtonTable[i].buttonPin) ? picoButtonTable[i].key : KEY_NULL;
    }
}

// Here's where we dissolved the st7789 driver into.
// Please note that most of this is copy-pasted from Pimoroni-Pico.
// Then it was translated over to C, because C++ had some libraries that are incompatible with C99.
uint8_t madctl;
uint16_t caset[2] = {0, 0};
uint16_t raset[2] = {0, 0};

typedef enum MADCTL {
    ROW_ORDER   = 0b10000000,
    COL_ORDER   = 0b01000000,
    SWAP_XY     = 0b00100000,  // AKA "MV"
    SCAN_ORDER  = 0b00010000,
    RGB_BGR     = 0b00001000,
    HORIZ_ORDER = 0b00000100
} __attribute__ ((__packed__)) MADCTL;

typedef enum reg {
    SWRESET   = 0x01,
    TEOFF     = 0x34,
    TEON      = 0x35,
    MADCTL    = 0x36,
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

static const unsigned int PIN_UNUSED = INT_MAX; // Intentionally INT_MAX to avoid overflowing MicroPython's int type

static const uint8_t SPI_DEFAULT_MOSI = 19;
static const uint8_t SPI_DEFAULT_MISO = 16;
static const uint8_t SPI_DEFAULT_DC = 16;
static const uint8_t SPI_DEFAULT_SCK = 18;

static const uint8_t SPI_BG_FRONT_PWM = 20;
static const uint8_t SPI_BG_FRONT_CS = 17;

static const uint8_t SPI_BG_BACK_PWM = 21;
static const uint8_t SPI_BG_BACK_CS = 22;
  
bool round;

// interface pins with our standard defaults where appropriate
spi_inst_t *spi;
uint8_t cs = SPI_BG_FRONT_CS;
uint8_t sck = SPI_DEFAULT_SCK;
uint8_t mosi = SPI_DEFAULT_MOSI;
uint8_t miso = SPI_DEFAULT_MISO;
uint8_t dc = SPI_DEFAULT_DC;
uint8_t bl = SPI_BG_FRONT_PWM;
uint8_t vsync  = PIN_UNUSED; // only available on some products
uint8_t parallel_sm;
PIO parallel_pio;
uint8_t parallel_offset;
uint8_t st_dma;

// The ST7789 requires 16 ns between SPI rising edges.
// 16 ns = 62,500,000 Hz
// 2350 doesn't support 62,500,000 so use 75,000,000 seems to work.
static const uint32_t SPI_BAUD = 75000000;

void command(uint8_t commandChar, int len, const char* data) {
    gpio_put(dc, 0); // command mode

    gpio_put(cs, 0);
    
    spi_write_blocking(spi, &commandChar, 1);

    if (data) {
        gpio_put(dc, 1); // data mode
        spi_write_blocking(spi, (const uint8_t*)data, len);
    }

    gpio_put(cs, 1);
}

inline void commandNoString(uint8_t commandChar) { command(commandChar, 0, ""); }

void SetBacklight(uint8_t brightness) {
    // gamma correct the provided 0-255 brightness value onto a
    // 0-65535 range for the pwm counter
    float gamma = 2.8;
    uint16_t value = (uint16_t)(pow((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
    pwm_set_gpio_level(bl, value);
  }

// And now expose this functionality to Raylib.
void InitDisplay(void)
{
    // First construct the pin information.
    // configure spi interface and pins
    spi_init(spi, SPI_BAUD);

    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);

    st_dma = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(st_dma);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_bswap(&config, false);
    channel_config_set_dreq(&config, spi_get_dreq(spi, true));
    dma_channel_configure(st_dma, &config, &spi_get_hw(spi)->dr, NULL, 0, false);

    // Then the full init
    gpio_set_function(dc, GPIO_FUNC_SIO);
    gpio_set_dir(dc, GPIO_OUT);

    gpio_set_function(cs, GPIO_FUNC_SIO);
    gpio_set_dir(cs, GPIO_OUT);

    // if a backlight pin is provided then set it up for
    // pwm control
    if(bl != PIN_UNUSED) {
      pwm_config cfg = pwm_get_default_config();
      pwm_set_wrap(pwm_gpio_to_slice_num(bl), 65535);
      pwm_init(pwm_gpio_to_slice_num(bl), &cfg, true);
      gpio_set_function(bl, GPIO_FUNC_PWM);
      set_backlight(0); // Turn backlight off initially to avoid nasty surprises
    }

    // Common init.
    commandNoString(SWRESET);

    sleep_ms(150);

    commandNoString(TEON);  // enable frame sync signal if used
    command(COLMOD,    1, "\x05");  // 16 bits per pixel

    command(PORCTRL, 5, "\x0c\x0c\x00\x33\x33");
    command(LCMCTRL, 1, "\x2c");
    command(VDVVRHEN, 1, "\x01");
    command(VRHS, 1, "\x12");
    command(VDVS, 1, "\x20");
    command(PWCTRL1, 2, "\xa4\xa1");
    command(FRCTRL2, 1, "\x0f");

    // As noted in https://github.com/pimoroni/pimoroni-pico/issues/1040
    // this is required to avoid a weird light grey banding issue with low brightness green.
    // The banding is not visible without tweaking gamma settings (GMCTRP1 & GMCTRN1) but
    // it makes sense to fix it anyway.
    command(RAMCTRL, 2, "\x00\xc0");

    // Pico Display 2.8 specific.
    command(GCTRL, 1, "\x35");
    command(VCOMS, 1, "\x1f");
    command(GMCTRP1, 14, "\xD0\x08\x11\x08\x0C\x15\x39\x33\x50\x36\x13\x14\x29\x2D");
    command(GMCTRN1, 14, "\xD0\x08\x10\x08\x06\x06\x39\x44\x51\x0B\x16\x14\x2F\x31");

    commandNoString(INVON);   // set inversion mode
    commandNoString(SLPOUT);  // leave sleep mode
    commandNoString(DISPON);  // turn display on

    sleep_ms(100);

    // Setup the bus boundaries.
    // Pico Display 2.0
    int width = (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT) ? 320 : 240;
    int height = (currentOrientation == PORTRAIT || currentOrientation == INVERTED_PORTRAIT) ? 240 : 320;

    if (width == 320 && height == 240) {
        caset[0] = 0;
        caset[1] = 319;
        raset[0] = 0;
        raset[1] = 239;
        madctl = (currentOrientation == INVERTED_LANDSCAPE || currentOrientation == PORTRAIT) ? ROW_ORDER : COL_ORDER;
        madctl |= SWAP_XY | SCAN_ORDER;
    }

    // Pico Display 2.0 at 90 degree rotation
    if (width == 240 && height == 320) {
        caset[0] = 0;
        caset[1] = 239;
        raset[0] = 0;
        raset[1] = 319;
        madctl = (currentOrientation == INVERTED_LANDSCAPE || currentOrientation == PORTRAIT) ? (COL_ORDER | ROW_ORDER) : 0;
    }

    // Byte swap the 16bit rows/cols values
    caset[0] = __builtin_bswap16(caset[0]);
    caset[1] = __builtin_bswap16(caset[1]);
    raset[0] = __builtin_bswap16(raset[0]);
    raset[1] = __builtin_bswap16(raset[1]);

    command(CASET,  4, (char *)caset);
    command(RASET,  4, (char *)raset);
    command(MADCTL, 1, (char *)&madctl);

    if(bl != PIN_UNUSED) {
        //update(); // Send the new buffer to the display to clear any previous content
        sleep_ms(50); // Wait for the update to apply
        SetBacklight(255); // Turn backlight on now surprises have passed
    }
}

void FlipBuffer(uint16_t* buffer)
{

}

void CleanupDisplay(void)
{
    if(dma_channel_is_claimed(st_dma)) {
        dma_channel_abort(st_dma);
        dma_channel_unclaim(st_dma);
    }
}