#include "vga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/scanvideo.h"
#include "pico/multicore.h"

static uint width = 320;
static uint height = 240;

// Note: frameBufferNext is set by Core 0
// Core 1 atomically swaps at scanline 0 when frameSwapPending is set
static const scanvideo_mode_t *vga_mode = NULL;

// Core 1 function to run scanvideo rendering loop
static void __not_in_flash_func(vga_core1_main)(void) {
    printf("[VGA] Core 1 started\n");
    fflush(stdout);
    
    // Initialize scanvideo on Core 1 (this core handles the video IRQs)
    if (vga_mode) {
        scanvideo_setup(vga_mode);
        scanvideo_timing_enable(true);
    }
    
    scanvideo_scanline_buffer_t *scanline_buffer = NULL;
    uint16_t last_frame = 0xFFFF;
    uint frame_count = 0;
    
    // TODO: actually draw the current buffer.  don't unlock that buffer until raylib says it's done with the next one.  
    while (true) {
        // Acquire a scanline buffer to fill
        scanline_buffer = scanvideo_begin_scanline_generation(true);
        
        uint16_t frame_id = scanvideo_frame_number(scanline_buffer->scanline_id);
        uint16_t scanline_id = scanvideo_scanline_number(scanline_buffer->scanline_id);
        
        // Framebuffer not yet set - render blank scanline
        uint16_t* tokens = (uint16_t*)scanline_buffer->data;
        tokens[0] = 3;      // TOKEN_COLOR_RUN
        tokens[1] = 0x0000; // Black
        tokens[2] = width - 3;
        tokens[3] = 1;      // TOKEN_EOL_ALIGN
        scanline_buffer->data_used = 2;
        
        scanline_buffer->status = SCANLINE_OK;
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

void InitVGA(unsigned int initWidth, unsigned int initHeight)
{
    width = initWidth;
    height = initHeight;
    
    printf("[VGA] Initializing with resolution %ux%u\n", width, height);
    
    // Pin configuration is handled by scanvideo library using compile-time macros:
    // PICO_SCANVIDEO_COLOR_PIN_BASE and PICO_SCANVIDEO_SYNC_PIN_BASE
    // These are set in the top-level CMakeLists.txt for Pimoroni_VGA_Demo
    
    // Select the appropriate scanvideo mode based on resolution
    // We do this manually to force clamps.
    if (width == 320 && height == 240) {
        vga_mode = &vga_mode_320x240_60;
    } else {
        printf("[VGA] Warning: Unsupported resolution %ux%u, defaulting to 320x240\n", width, height);
        vga_mode = &vga_mode_320x240_60;
        width = 320;
        height = 240;
    }
    
    // NOTE: scanvideo_setup() and Core 1 launch deferred to VGAStartCore1()
    // This ensures rlsw buffer allocation completes before scanvideo initialization
}

void VGAStartCore1(void)
{
    if (!vga_mode) {
        printf("[VGA] Error: VGAStartCore1() called before InitVGA()\n");
        return;
    }
    
    printf("[VGA] Starting Core 1 rendering\n");
    
    // Launch Core 1 to run the video output
    // Core 1 will call scanvideo_setup() and handle video IRQs
    multicore_launch_core1(vga_core1_main);
}

void FlipVGAFrameBuffer(uint16_t* framebuffer)
{
    // TODO: Set the pending framebuffer pointer
    
}

void CleanupVGA(void)
{
    // Kill Core 1 task
    multicore_reset_core1();
}