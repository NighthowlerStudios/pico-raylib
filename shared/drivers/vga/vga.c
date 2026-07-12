#include "vga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/scanvideo.h"
#include "pico/multicore.h"

static uint width = 320;
static uint height = 240;

// Note: frameBufferNext is set by Core 0 via FIFO
// Core 1 receives it at frame boundaries for stable frame-locked synchronization
static const scanvideo_mode_t *vga_mode = NULL;

// Core 1 function to run scanvideo rendering loop
static void __not_in_flash_func(vga_core1_main)(void) {    
    // Initialize scanvideo on Core 1 (this core handles the video IRQs)
    if (vga_mode) {
        scanvideo_setup(vga_mode);
        scanvideo_timing_enable(true);
    }
    
    scanvideo_scanline_buffer_t *scanline_buffer = NULL;
    uint16_t last_frame = 0xFFFF;
    uint16_t* current_render_buffer = NULL;  // Buffer being rendered for current frame
    
    // TODO: actually draw the current buffer.  don't unlock that buffer until raylib says it's done with the next one.  
    while (true) {
        // Acquire a scanline buffer to fill
        scanline_buffer = scanvideo_begin_scanline_generation(true);
        
        uint16_t frame_id = scanvideo_frame_number(scanline_buffer->scanline_id);
        uint16_t scanline_id = scanvideo_scanline_number(scanline_buffer->scanline_id);
        
        // Swap buffers at frame boundaries (frame_id only changes at scanline 0)
        if (frame_id != last_frame) {
            last_frame = frame_id;
            
            // Check for new buffer from Core 0 at frame boundary
            if (multicore_fifo_rvalid()) {
                // Accept the new buffer
                current_render_buffer = (uint16_t*)multicore_fifo_pop_blocking();
                // Send acknowledgment back to Core 0 that we've accepted this buffer at scanline 0
                multicore_fifo_push_blocking(1);
            }
        }
        
        uint16_t* tokens = (uint16_t*)scanline_buffer->data;
        uint32_t token_idx = 0;
        
        // If framebuffer is available, render it; otherwise render blank scanline
        if (current_render_buffer) {
            // Calculate the offset into the framebuffer for this scanline
            // OpenGL buffers are vertically flipped, so read from bottom to top
            uint32_t fb_scanline_id = height - 1 - scanline_id;
            uint32_t scanline_offset = fb_scanline_id * width;
            uint16_t* scanline_data = current_render_buffer + scanline_offset;
            
            // Use TOKEN_RAW_RUN to encode the entire scanline as individual pixels
            // This avoids complexity with multiple COLOR_RUN tokens
            tokens[token_idx++] = 7;      // TOKEN_RAW_RUN
            tokens[token_idx++] = width;  // Number of pixels
            
            // Copy all pixel data directly using memcpy with 32-bit alignment
            memcpy(&tokens[token_idx], scanline_data, (width / 2) * sizeof(uint32_t));
            token_idx += width;
            
            // Add TOKEN_EOL_ALIGN in the high halfword
            tokens[token_idx++] = 1;  // TOKEN_EOL_ALIGN
            
            // data_used is number of 32-bit words (token pairs), rounded up
            scanline_buffer->data_used = (token_idx + 1) / 2;
        } else {
            // Framebuffer not yet set - render blank scanline
            tokens[0] = 7;      // TOKEN_RAW_RUN
            tokens[1] = width;  // Number of pixels
            // Fill rest with black pixels using memset with 32-bit alignment
            memset(&tokens[2], 0x00000000, (width / 2) * sizeof(uint32_t));
            tokens[2 + width] = 1;  // TOKEN_EOL_ALIGN
            scanline_buffer->data_used = (2 + width + 1 + 1) / 2;  // +1 for EOL
        }
        
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
    
    // TODO: This might be wrong.  we really need scanlines to be processed in SRAM.
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
    // Send framebuffer pointer to Core 1 via FIFO
    // This will block if FIFO is full
    multicore_fifo_push_blocking((uintptr_t)framebuffer);
    
    // Wait for Core 1 acknowledgment that it has accepted this buffer at scanline 0
    // This ensures Core 1 has switched to rendering the new buffer before we allow
    // raylib to modify the old buffer
    multicore_fifo_pop_blocking();
}

void CleanupVGA(void)
{
    // Kill Core 1 task
    multicore_reset_core1();
}