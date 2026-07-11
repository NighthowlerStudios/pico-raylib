#include <stdio.h>
#include "pico/stdlib.h"

// Use priority so this init occurs before filesystem does.  That way we can capture the logs.
void InitPicoRaylib(void) __attribute__((constructor(1000)));

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
    // We don't need runtime code in here anymore to set core and voltage because CMake is doing it.

    // Explicitly bind the peripheral subsystem directly to the 250MHz sys_clk source
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

    // TODO: Allow PC to communicate with Flash on BOOTSEL, then unfreeze raylib after BOOTSEL again.
}