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

// The ST7789 requires 16 ns between SPI rising edges.
// 16 ns = 62,500,000 Hz
// We are write only, so going past 62,500,000 is a safe overclock.
static const uint32_t SPI_BAUD = 75000000;

// Write to the SPI using the assigned DMA channel.
void command(uint8_t commandChar, int len, const char* data) {
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
void commandNoString(uint8_t commandChar) 
{ 
    command(commandChar, 0, NULL); 
}

// DMA variant of command for large data (framebuffer)
// Starts DMA transfer and returns immediately without blocking
void command_dma(uint8_t commandChar, int len, const char* data) {
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

#ifdef MULTICORE_VIDEO_OUTPUT

// We use lazy scheduling in here because the framebuffer is read-only on core 2 anyway.
#include "pico/mutex.h"
#include "pico/multicore.h"

static mutex_t frameBufferMutex;

void Core1FlipBuffer(void)
{
    // TODO: LockDMA().  The problem is framerates are unstable with it.
    // There is very little use of putting video over here on SPI displays.  Consider this approach abandoned.
    while(true)
    {
        // Pop buffer OUTSIDE mutex to avoid deadlock
        uintptr_t buffer_ptr = (uintptr_t)multicore_fifo_pop_blocking();
        uint32_t dimensions = multicore_fifo_pop_blocking();
        int currentWidth = (int)(dimensions >> 16);
        int currentHeight = (int)(dimensions & 0xffff);
        uint16_t* currentBuffer = (uint16_t*)buffer_ptr;

        //printf("[DEVICE] Current Buffer Pointer: %x\n", currentBuffer);

        // Core 1 owns the mutex while managing the DMA transfer
        mutex_enter_blocking(&frameBufferMutex);
        
        // Start 
        command(RAMWR, currentWidth * currentHeight * sizeof(uint16_t), (const char*)currentBuffer);
        
        mutex_exit(&frameBufferMutex);
    }
}

#endif

void InitST7789(uint16_t width, uint16_t height, uint8_t mosi, uint8_t dc, uint8_t sck, uint8_t pwm, uint8_t cs, bool circular)
{
    DC = dc;
    SCK = sck;
    MOSI = mosi;
    PWM = pwm;
    CS = cs;

    sleep_ms(100);

    // First construct the pin information.
    // configure spi interface and pins
    spi_init(spi, SPI_BAUD);

    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    printf("[ST7789] Claiming DMA Channel for LCD SPI.\n");

    // Don't consume this channel on the wrong core.
    // Otherwise, only enable DMA if we have a backbuffer.
#if !defined(MULTICORE_VIDEO_OUTPUT) && defined(RLSW_BACKBUFFER)
    LockDMA();
#endif

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
    // OpenGL draws its coordinates bottom to top instead of top to bottom so we need to override some stuff in here.
    uint8_t madctl;
    uint16_t caset[2] = {0, 0};
    uint16_t raset[2] = {0, 0};

    // 240x240 Square and Round LCD Breakouts
    if(width == 240 && height == 240) {
      int row_offset = circular ? 40 : 80;
      int col_offset = 0;
    
      switch(currentOrientation) {
        case PORTRAIT:
          if (!circular) row_offset = 0;
          caset[0] = row_offset;
          caset[1] = width + row_offset - 1;
          raset[0] = col_offset;
          raset[1] = width + col_offset - 1;

          madctl = ROW_ORDER;
          break;
        case INVERTED_LANDSCAPE:
          caset[0] = col_offset;
          caset[1] = width + col_offset - 1;
          raset[0] = row_offset;
          raset[1] = width + row_offset - 1;

          madctl = SWAP_XY | COL_ORDER | ROW_ORDER;
          break;
        case INVERTED_PORTRAIT:
          caset[0] = row_offset;
          caset[1] = width + row_offset - 1;
          raset[0] = col_offset;
          raset[1] = width + col_offset - 1;

          madctl = COL_ORDER;
          break;
        default: // ROTATE_0 (and for any smart-alec who tries to rotate 45 degrees or something...)
          if (!circular) row_offset = 0;
          caset[0] = col_offset;
          caset[1] = width + col_offset - 1;
          raset[0] = row_offset;
          raset[1] = width + row_offset - 1;

          madctl = SWAP_XY;
          break;
      }
    }

    // Pico Display
    if(width == 240 && height == 135) {
      caset[0] = 40;   // 240 cols
      caset[1] = 40 + width - 1;
      raset[0] = 52;   // 135 rows
      raset[1] = 52 + height - 1;
      if (currentOrientation == INVERTED_LANDSCAPE) {
        raset[0] += 1;
        raset[1] += 1;
      }
      madctl = currentOrientation == INVERTED_LANDSCAPE ? ROW_ORDER | COL_ORDER : 0;
      madctl |= SWAP_XY;
    }

    // Pico Display at 90 degree rotation
    if(width == 135 && height == 240) {
      caset[0] = 52;   // 135 cols
      caset[1] = 52 + width - 1;
      raset[0] = 40;   // 240 rows
      raset[1] = 40 + height - 1;
      madctl = 0;
      if (currentOrientation == INVERTED_PORTRAIT) {
        caset[0] += 1;
        caset[1] += 1;
      }
      madctl = currentOrientation == INVERTED_PORTRAIT ? COL_ORDER : ROW_ORDER;
    }

    // Pico Display 2.0
    if(width == 320 && height == 240) {
      caset[0] = 0;
      caset[1] = 319;
      raset[0] = 0;
      raset[1] = 239;
      madctl = currentOrientation == INVERTED_LANDSCAPE ? ROW_ORDER | COL_ORDER : 0;
      madctl |= SWAP_XY;
    }

    // Pico Display 2.0 at 90 degree rotation
    if(width == 240 && height == 320) {
      caset[0] = 0;
      caset[1] = 239;
      raset[0] = 0;
      raset[1] = 319;
      madctl = currentOrientation == INVERTED_PORTRAIT ? COL_ORDER : ROW_ORDER;
    }

    // Byte swap the 16bit rows/cols values
    caset[0] = __builtin_bswap16(caset[0]);
    caset[1] = __builtin_bswap16(caset[1]);
    raset[0] = __builtin_bswap16(raset[0]);
    raset[1] = __builtin_bswap16(raset[1]);

    command(CASET,  4, (char *)caset);
    command(RASET,  4, (char *)raset);
    command(MASPI_DEFAULT_DCTLREG, 1, (char *)&madctl);

    printf("[ST7789] Ready for use.\n");

    if(PWM != PIN_UNUSED) {
        //update(); // Send the new buffer to the display to clear any previous content
        sleep_ms(50); // Wait for the update to apply
        SetBacklight(255); // Turn backlight on now surprises have passed
    }

    // Create the mutex and initialize Core 1.
#ifdef MULTICORE_VIDEO_OUTPUT
    if (width * height > 320 * 240)
    {
        printf("[ST7789] [WARNING] LCD SPI is set to use Core 1 instead of 0, but on this display resolution, one or more buffers will likely end up in PSRAM.\n");
    }

    printf("[ST7789] Setting up Core 1 as SPI instead of Core 0...\n");

    mutex_init(&frameBufferMutex);
    multicore_reset_core1();
    multicore_launch_core1(Core1FlipBuffer);
#else
#if RLSW_BACKBUFFER
    printf("[ST7789] Using DMA to transmit the framebuffer asynchronously.\n");
#else
    printf("[ST7789][WARNING] LCD SPI is set to use the same core as Raylib, without asynchronous DMA.  Memory is saved but performance will suffer with extreme overhead.\n");
#endif
#endif
}

void SendBufferST7789(int width, int height, const char* buffer)
{
        // Raylib must wait until Core 1 is done transferring the previous buffer.
#ifdef MULTICORE_VIDEO_OUTPUT
    //printf("[DEVICE] Frame buffer flipped.  Waiting until Core 1 is done using SPI...\n");
    // We can't just block mutex above launch_core1, because we would deadlock up there at the other enter_blocking
    mutex_enter_blocking(&frameBufferMutex);
    //printf("[DEVICE] Telling Core 1 about the new pointer.\n");

    uintptr_t buffer_ptr = (uintptr_t)buffer;
    uint32_t dimensions = ((uint32_t)width << 16) | (uint32_t)height;
    multicore_fifo_push_blocking(buffer_ptr);
    multicore_fifo_push_blocking(dimensions);

    mutex_exit(&frameBufferMutex);
#else    
        // Takes 0.01952 seconds on average 320 x 240 lcd's, but draw time of raylib is quite high.  
        // If you overclock to 250MHz, raylib time is very quick, but SPI raises to around 0.023404
        // The hard cap for the entire transfer is around 43fps, even if DMA is enabled to async it.
    #ifdef RLSW_BACKBUFFER
        // Wait for previous DMA transfer to complete if raylib is faster than the display
        // Note: is_busy is unreliable, so we poll transfer_count instead
        dma_channel_hw_t *hw = dma_channel_hw_addr(st_dma);
        while (hw->transfer_count > 0) {
            tight_loop_contents();
        }
        gpio_put(CS, 1);
        command_dma(RAMWR, width * height * sizeof(uint16_t), buffer);
    #else
        command(RAMWR, width * height * sizeof(uint16_t), buffer);
    #endif
    
#endif
}

void CleanupST7789(void)
{
    // Don't unallocate dma's until the current frame is done drawing.
#ifdef MULTICORE
    mutex_enter_blocking(&frameBufferMutex);
    // No-op Core 1.
    multicore_reset_core1();
#endif

    if(dma_channel_is_claimed(st_dma)) {
        dma_channel_abort(st_dma);
        dma_channel_unclaim(st_dma);
    }

    spi_deinit(spi);

    // Conserve power.
    
    SetBacklight(0);
    commandNoString(DISPOFF);
    commandNoString(SLPIN);

    printf("[ST7789] Screen, SPI and DMA destroyed.");
}