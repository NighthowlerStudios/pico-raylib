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

#include "st7789.h"

#include <limits.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Here's where we dissolved the st7789 driver into.
// Please note that most of this is copy-pasted from Pimoroni-Pico.
// Then it was translated over to C, because C++ had some libraries that are incompatible with C99.

static const uint8_t PIN_UNUSED = CHAR_MAX; // Intentionally INT_MAX to avoid overflowing MicroPython's int type

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

#include "hardware/spi.h"

// interface pins with our standard defaults where appropriate
static spi_inst_t *spi = spi0;
static uint8_t parallel_sm;
static PIO parallel_pio;
static uint8_t parallel_offset;
static uint8_t st_dma;

static uint8_t DC;
static uint8_t CS;
static uint8_t PWM;
static uint8_t MOSI;
static uint8_t SCK;

void WaitForDMA(void)
{
    dma_channel_wait_for_finish_blocking(st_dma);
    gpio_put(CS, 1);
}

// Write to the SPI using the assigned DMA channel.
void CommandSPIBlocking(uint8_t commandChar, int len, const char* data) {
    gpio_put(DC, 0); // command mode

    gpio_put(CS, 0);
    
    spi_write_blocking(spi, &commandChar, 1);

    if (data) {
        gpio_put(DC, 1); // data mode
        spi_write_blocking(spi, (const uint8_t*)data, len);
    }

    gpio_put(CS, 1);
}

// command but with a NULL set of data.
void CommandNoString(uint8_t commandChar) 
{ 
    CommandSPIBlocking(commandChar, 0, NULL); 
}

// DMA variant of command for large data (framebuffer)
// Starts DMA transfer and returns immediately without blocking
void CommandDMA(uint8_t commandChar, int len, const char* data) {
    gpio_put(DC, 0); // command mode
    gpio_put(CS, 0);
    
    // Send command byte using blocking SPI (small, so it's fine)
    spi_write_blocking(spi, &commandChar, 1);
    
    if (data && len > 0) {
        gpio_put(DC, 1); // data mode
        
        // DMA is already configured in InitST7789(), just update the source and count
        dma_channel_set_read_addr(st_dma, data, false);
        dma_channel_set_trans_count(st_dma, len, true);  // true = start immediately
        
        // Return immediately - DMA runs in background
    } else {
        // No data to transfer, release CS immediately
        gpio_put(CS, 1);
    }
}

#include <math.h>

void SetBacklight(uint8_t brightness) {
    // gamma correct the provided 0-255 brightness value onto a
    // 0-65535 range for the pwm counter
    float gamma = 2.8;
    uint16_t value = (uint16_t)(pow((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
    pwm_set_gpio_level(PWM, value);
}

void LockDMA()
{
    // Hand ownership of the DMA channel to the caller.
    st_dma = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(st_dma);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_bswap(&config, false);
    channel_config_set_dreq(&config, spi_get_dreq(spi, true));
    dma_channel_configure(st_dma, &config, &spi_get_hw(spi)->dr, NULL, 0, false);
}

void ResizeWindowST7789(uint16_t width, uint16_t height, bool circular)
{
    // Setup the bus boundaries.
    // OpenGL draws its coordinates bottom to top instead of top to bottom so we need to override some stuff in here.
    uint8_t madctl;
    uint16_t caset[2] = {0, 0};
    uint16_t raset[2] = {0, 0};

    // TODO: check round displays for int row_offset = circular ? 40 : 80;

    // ST7789 max res is 320x240
    // Always center on the physical 320x240 screen
    int col_offset = (320 - width) / 2;
    int row_offset = (240 - height) / 2;

    // add one if the width or height is odd, to center the image on the display
    if ((width % 2) != 0) {
        col_offset += 1;
    }
    if ((height % 2) != 0) {
        row_offset += 1;
    }

    caset[0] = col_offset;
    caset[1] = width + col_offset - 1;
    raset[0] = row_offset;
    raset[1] = height + row_offset - 1;

    switch(currentOrientation) {
        case PORTRAIT:
            madctl = ROW_ORDER;
            break;
        case INVERTED_LANDSCAPE:
            madctl = SWAP_XY | COL_ORDER | ROW_ORDER;
            break;
        case INVERTED_PORTRAIT:
            madctl = COL_ORDER;
            break;
        default:
            madctl = SWAP_XY;
            break;
    }

    // Byte swap the 16bit rows/cols values
    caset[0] = __builtin_bswap16(caset[0]);
    caset[1] = __builtin_bswap16(caset[1]);
    raset[0] = __builtin_bswap16(raset[0]);
    raset[1] = __builtin_bswap16(raset[1]);

    CommandSPIBlocking(CASET,  4, (char *)caset);
    CommandSPIBlocking(RASET,  4, (char *)raset);
    CommandSPIBlocking(MASPI_DEFAULT_DCTLREG, 1, (char *)&madctl);
}

void CommandClearBlack() {
    ResizeWindowST7789(320, 240, false); // Ensure the window is set to the full size of display RAM.  Let the controller ignore the rest.

    gpio_put(DC, 0); // command mode

    gpio_put(CS, 0);
    
    uint8_t commandChar = RAMWR;
    spi_write_blocking(spi, &commandChar, 1);

    gpio_put(DC, 1); // data mode
    uint8_t black = 0x00;
    for (int i = 0; i < 320 * 240 * sizeof(uint16_t); i++) {
        spi_write_blocking(spi, &black, 1);
    }

    gpio_put(CS, 1);
}

void InitST7789(uint16_t width, uint16_t height, uint8_t mosi, uint8_t dc, uint8_t sck, uint8_t pwm, uint8_t cs, bool circular)
{
    DC = dc;
    SCK = sck;
    MOSI = mosi;
    PWM = pwm;
    CS = cs;

    sleep_ms(100);
    printf("[ST7789] Initializing ST7789 with protocol speed %i Hz\n", SPI_BAUD);

    // First construct the pin information.
    // configure spi interface and pins
    spi_init(spi, SPI_BAUD);

    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    printf("[ST7789] Claiming DMA Channel for LCD SPI.\n");

    gpio_set_function(DC, GPIO_FUNC_SIO);
    gpio_set_dir(DC, GPIO_OUT);

    gpio_set_function(CS, GPIO_FUNC_SIO);
    gpio_set_dir(CS, GPIO_OUT);

    printf("[ST7789] Backlight off (PWM init)...\n");

    // if a backlight pin is provided then set it up for
    // pwm control
    if(PWM != PIN_UNUSED) {
        pwm_config cfg = pwm_get_default_config();
        pwm_set_wrap(pwm_gpio_to_slice_num(PWM), 65535);
        pwm_init(pwm_gpio_to_slice_num(PWM), &cfg, true);
        gpio_set_function(PWM, GPIO_FUNC_PWM);
        SetBacklight(0); // Turn backlight off initially to avoid nasty surprises
    }

    // Then the full init

    // Common init.
    printf("[ST7789] Boot the ST7789\n");

    CommandNoString(SWRESET);

    CommandNoString(TEON);  // enable frame sync signal if used
    CommandSPIBlocking(COLMOD,    1, "\x05");  // 16 bits per pixel

    const char porctrl_bytes[] = {0x0c, 0x0c, 0x00, 0x33, 0x33};
    CommandSPIBlocking(PORCTRL, 5, &porctrl_bytes);
    CommandSPIBlocking(LCMCTRL, 1, "\x2c");
    CommandSPIBlocking(VDVVRHEN, 1, "\x01");
    CommandSPIBlocking(VRHS, 1, "\x12");
    CommandSPIBlocking(VDVS, 1, "\x20");
    const char pwctrl_bytes[] = {0xa4, 0xa1};
    CommandSPIBlocking(PWCTRL1, 2, &pwctrl_bytes);
    CommandSPIBlocking(FRCTRL2, 1, "\x0f");

    // As noted in https://github.com/pimoroni/pimoroni-pico/issues/1040
    // this is required to avoid a weird light grey banding issue with low brightness green.
    // The banding is not visible without tweaking gamma settings (GMCTRP1 & GMCTRN1) but
    // it makes sense to fix it anyway.
    const char ramctrl_bytes[] = { 0x00, 0xc0 };
    CommandSPIBlocking(RAMCTRL, 2, &ramctrl_bytes);

    // Pico Display 2.8 specific.
    CommandSPIBlocking(GCTRL, 1, "\x35");
    CommandSPIBlocking(VCOMS, 1, "\x1f");
    const char gmctrp1_bytes[] = {0xd0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2d };
    const char gmctrn1_bytes[] = {0xd0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0b, 0x16, 0x14, 0x2f, 0x31 };
    CommandSPIBlocking(GMCTRP1, 14, &gmctrp1_bytes);
    CommandSPIBlocking(GMCTRN1, 14, &gmctrn1_bytes);

    CommandNoString(INVON);   // set inversion mode
    CommandNoString(SLPOUT);  // leave sleep mode
    CommandNoString(DISPON);  // turn display on

    sleep_ms(100);

    // Use these two to clear the entire device before actually setting the buffer size to the intended target.
    CommandClearBlack(320 * 240 * sizeof(uint16_t)); // Clear the screen to black
    ResizeWindowST7789(width, height, circular);

    printf("[ST7789] Ready for use.\n");

    if(PWM != PIN_UNUSED) {
        //update(); // Send the new buffer to the display to clear any previous content
        sleep_ms(50); // Wait for the update to apply
        SetBacklight(255); // Turn backlight on now surprises have passed
    }

#if SW_DOUBLE_BUFFERING
    printf("[ST7789] Using DMA to transmit the framebuffer asynchronously.\n");
    LockDMA();
#else
    printf("[ST7789][WARNING] LCD SPI is set to use the same core as Raylib, without asynchronous DMA.  Memory is saved but performance will suffer with extreme overhead.\n");
#endif
}

void SendBufferST7789(int width, int height, const char* buffer)
{
    // Raylib must wait until DMA is done transferring the previous buffer.   
    // Takes 0.01952 seconds on average 320 x 240 lcd's, but draw time of raylib is quite high.  
    // If you overclock to 240MHz, raylib time is very quick, but SPI raises to around 0.024323
    // The hard cap for the entire transfer is around 41fps, even if DMA is enabled to async it.
#ifdef SW_DOUBLE_BUFFERING
    // Wait for previous DMA transfer to complete if raylib is faster than the display
    WaitForDMA();
    gpio_put(CS, 1);
    CommandDMA(RAMWR, width * height * sizeof(uint16_t), buffer);
#else
    CommandSPIBlocking(RAMWR, width * height * sizeof(uint16_t), buffer);
#endif
}

void CleanupST7789(void)
{
    if(dma_channel_is_claimed(st_dma)) {
        dma_channel_abort(st_dma);
        dma_channel_unclaim(st_dma);
    }

    spi_deinit(spi);

    // Conserve power.
    
    SetBacklight(0);
    CommandNoString(DISPOFF);
    CommandNoString(SLPIN);

    printf("[ST7789] Screen, SPI and DMA destroyed.");
}