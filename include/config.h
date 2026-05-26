/**********************************************************************************************
*
*   raylib configuration flags - Pico2 Embedded Platform
*
*   Custom configuration for embedded Pico2 targets with limited resources
*
**********************************************************************************************/

// Clone of https://github.com/georgik/esp-idf-component-raylib/blob/main/raylib/include/config.h

#ifndef CONFIG_H
#define CONFIG_H

// Include Pico SDK headers for memory allocation.  TODO

// Configure software renderer to use RGB565 internally (like Raylib 5.6.0)
// This avoids format conversion and matches the working version
#define SW_FRAMEBUFFER_COLOR_TYPE R5G6B5
#define SW_FRAMEBUFFER_OUTPUT_BGRA false

//------------------------------------------------------------------------------------
// Module selection - Minimal set for embedded 2D rendering
//------------------------------------------------------------------------------------
#define SUPPORT_MODULE_RSHAPES          1
#define SUPPORT_MODULE_RTEXTURES        1
#define SUPPORT_MODULE_RTEXT            1
#define SUPPORT_MODULE_RMODELS          1 
#define SUPPORT_MODULE_RAUDIO           1

//------------------------------------------------------------------------------------
// Module: rcore - Configuration Flags
//------------------------------------------------------------------------------------
#define SUPPORT_CAMERA_SYSTEM           1
#define SUPPORT_GESTURES_SYSTEM         1       // Touch gestures enabled
#define SUPPORT_RPRAND_GENERATOR        1
#define SUPPORT_MOUSE_GESTURES          0       // No mouse on embedded
#define SUPPORT_SSH_KEYBOARD_RPI        0
#define SUPPORT_WINMM_HIGHRES_TIMER     0
#define SUPPORT_PARTIALBUSY_WAIT_LOOP   0
#define SUPPORT_SCREEN_CAPTURE          0       // Disabled for embedded
#define SUPPORT_GIF_RECORDING           0       // Disabled for embedded
#define SUPPORT_COMPRESSION_API         0       // Disabled for embedded
#define SUPPORT_AUTOMATION_EVENTS       0       // Disabled for embedded
#define SUPPORT_CLIPBOARD_IMAGE         0       // Disabled for embedded

//------------------------------------------------------------------------------------
// rcore: Configuration values
//------------------------------------------------------------------------------------
#define MAX_FILEPATH_CAPACITY        512        // Reduced for embedded
#define MAX_FILEPATH_LENGTH          256        // Reduced for embedded

#define MAX_KEYBOARD_KEYS             128       // Reduced for embedded
#define MAX_MOUSE_BUTTONS               4
#define MAX_GAMEPADS                    1       // Reduced for embedded
#define MAX_GAMEPAD_AXES                4
#define MAX_GAMEPAD_BUTTONS            16
#define MAX_GAMEPAD_VIBRATION_TIME      2.0f
#define MAX_TOUCH_POINTS                4       // Reduced for embedded
#define MAX_KEY_PRESSED_QUEUE           8       // Reduced for embedded
#define MAX_CHAR_PRESSED_QUEUE          8       // Reduced for embedded

#define MAX_DECOMPRESSION_SIZE         16       // Reduced for embedded (16 MB)

//------------------------------------------------------------------------------------
// Module: rlgl - Configuration values
//------------------------------------------------------------------------------------
#define RL_SUPPORT_MESH_GPU_SKINNING           0      // Disabled for embedded

// Undefine and redefine rlgl values for embedded systems
#undef RL_DEFAULT_BATCH_BUFFERS
#define RL_DEFAULT_BATCH_BUFFERS               1

#undef RL_DEFAULT_BATCH_DRAWCALLS
#define RL_DEFAULT_BATCH_DRAWCALLS           128      // Reduced for embedded

#undef RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS
#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS     2      // Reduced for embedded

#undef RL_MAX_MATRIX_STACK_SIZE
#define RL_MAX_MATRIX_STACK_SIZE              16      // Reduced for embedded

#undef RL_MAX_SHADER_LOCATIONS
#define RL_MAX_SHADER_LOCATIONS               16      // Reduced for embedded

#undef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR              0.05

#undef RL_CULL_DISTANCE_FAR
#define RL_CULL_DISTANCE_FAR            1000.0        // Reduced for embedded

//------------------------------------------------------------------------------------
// Module: rshapes - Configuration Flags
//------------------------------------------------------------------------------------
#define SUPPORT_QUADS_DRAW_MODE         1

//------------------------------------------------------------------------------------
// rshapes: Configuration values
//------------------------------------------------------------------------------------
#define SPLINE_SEGMENT_DIVISIONS       16       // Reduced for embedded

//------------------------------------------------------------------------------------
// Module: rtextures - Configuration Flags
//------------------------------------------------------------------------------------
#define SUPPORT_FILEFORMAT_PNG      1
#define SUPPORT_FILEFORMAT_BMP      1
#define SUPPORT_FILEFORMAT_TGA      1
#define SUPPORT_FILEFORMAT_JPG      1
#define SUPPORT_FILEFORMAT_GIF      1
#define SUPPORT_FILEFORMAT_QOI      1
#define SUPPORT_FILEFORMAT_PSD      0
#define SUPPORT_FILEFORMAT_DDS      1
#define SUPPORT_FILEFORMAT_HDR      0
#define SUPPORT_FILEFORMAT_PIC      0
#define SUPPORT_FILEFORMAT_KTX      1
#define SUPPORT_FILEFORMAT_ASTC     1

//------------------------------------------------------------------------------------
// Module: rtextures - Configuration values
//------------------------------------------------------------------------------------
#define MAX_IMAGE_FLIP_QUEUE            2       // Reduced for embedded

//------------------------------------------------------------------------------------
// Module: rtext - Configuration Flags
//------------------------------------------------------------------------------------
#define SUPPORT_FILEFORMAT_FNT          1
#define SUPPORT_FILEFORMAT_TTF          0       // Disabled - too heavy for embedded
#define SUPPORT_FILEFORMAT_BMF          0

//------------------------------------------------------------------------------------
// rtext: Configuration values
//------------------------------------------------------------------------------------
#define MAX_TEXT_BUFFER_LENGTH        512       // Reduced for embedded
#define MAX_TEXTSPLIT_COUNT            64       // Reduced for embedded

#endif // CONFIG_H