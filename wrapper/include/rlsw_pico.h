/**********************************************************************************************
*
*   rlsw_esp_idf.h - Override rlsw memory allocators and configure for ESP-IDF
*
*   This header must be included BEFORE rlsw.h to override defaults
*
**********************************************************************************************/

#ifndef RLSW_PICO_H
#define RLSW_PICO_H

// Configure framebuffer copy format for RGB (not BGR)
// Most Pico display libraries expect RGB565 format
#define SW_GL_FRAMEBUFFER_COPY_BGRA false

#endif // RLSW_PICO_H