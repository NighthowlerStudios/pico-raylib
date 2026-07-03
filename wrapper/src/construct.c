#include <stdio.h>
#include "pico/stdlib.h"
#include "filesystem/vfs.h"

void InitPicoRaylib(void) __attribute__((constructor));

// Declare fs_init from pico-vfs (weak symbol)
extern bool fs_init(void);

#ifdef OVERCLOCK
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#endif

#include "pico_display.h"

#ifndef RGB_LED_IMPLEMENTED
    #define SHOW_LED_WAITING_FOR_USB
    #define SHOW_NO_LED
#endif

void InitPicoRaylib(void)
{
#ifdef OVERCLOCK

#warning "Overclocking is active.  This will set the overclock detect bit on your device and void your warranty.  DO NOT INSTALL IF YOU WANT YOUR WARRANTY."
    // We use these numbers to show competition with the ESP32, which has a 240MHz clock.

    vreg_set_voltage(VREG_VOLTAGE_1_20);
    set_sys_clock_khz(250000, true); // This clock was chosen as it prevents the SPI clock from dividing badly.
    sleep_ms(200); // For stability of the voltage regulator.

    // Force the peripheral clock to be half the CPU. This will also force SPI to be as high as it can go.
    // Consequently, we prevent overclocks past 250mhz.
    uint32_t freq = clock_get_hz(clk_sys);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);

#endif

    stdio_init_all();

#ifdef USE_USB_CONSOLE_OUT
    // Using the same pins does not matter.
#if defined(USE_RGB_LED_AS_DEBUG) && defined(RGB_LED_IMPLEMENTED)
    rgb = InitRGBLED(PICO_DISPLAY_LED_R, PICO_DISPLAY_LED_G, PICO_DISPLAY_LED_B);
#endif

    while (!stdio_usb_connected())
    {
        SHOW_LED_WAITING_FOR_USB;
        sleep_ms(250);
        SHOW_NO_LED;
        sleep_ms(250);
    }
#endif

    printf("[DEVICE] Pico System initialised.\n");

    // Initialize filesystem after stdio is ready
    fs_init();

#ifdef SD_CARD
    // Overclock is stable now.  Init SD card mounts.

#endif

    // TODO: Allow PC to communicate with Flash on BOOTSEL, then unfreeze raylib after BOOTSEL again.
}