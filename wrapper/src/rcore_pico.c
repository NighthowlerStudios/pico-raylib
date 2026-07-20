/**********************************************************************************************
*
*   rcore_<platform> template - Functions to manage window, graphics device and inputs
*
*   PLATFORM: <PLATFORM>
*       - TODO: Define the target platform for the core
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - <platform-specific SDK dependency>
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5) and contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/
#include "config.h"         // Must be first to set SW_FRAMEBUFFER_COLOR_TYPE before rlgl.h
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

// All display devices are interfaced to this single header.  
// Only one optional library is selected at a time in CMakeLists.txt
#include "pico_display.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    int placeholder;

} PlatformData;

#define MAX_TRACELOG_MSG_LENGTH      256        // Max length of one trace-log message
#define MAX_GAMEPAD_NAME_LENGTH      128        // Maximum number of characters in a gamepad name (byte size)

#define MAX_AUTOMATION_EVENTS      16384        // Maximum number of automation events to record

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Found in rcore.c
typedef struct { int x; int y; } Point;
typedef struct { unsigned int width; unsigned int height; } Size;

typedef struct CoreData {
    struct {
        const char *title;                  // Window text title const pointer
        unsigned int flags;                 // Configuration flags (bit based), keeps window state
        bool ready;                         // Check if window has been initialized successfully
        bool shouldClose;                   // Check if window set for closing
        bool resizedLastFrame;              // Check if window has been resized last frame
        bool eventWaiting;                  // Wait for events before ending frame
        bool usingFbo;                      // Using FBO (RenderTexture) for rendering instead of default framebuffer

        Size display;                       // Display width and height (monitor, device-screen, LCD, ...)
        Size screen;                        // Screen current width and height
        Point position;                     // Window current position
        Size previousScreen;                // Screen previous width and height (required on fullscreen/borderless-windowed toggle)
        Point previousPosition;             // Window previous position (required on fullscreen/borderless-windowed toggle)
        Size render;                        // Screen framebuffer width and height
        Point renderOffset;                 // Screen framebuffer render offset (Not required anymore?)
        Size currentFbo;                    // Current framebuffer render width and height (depends on active render texture)
        Size screenMin;                     // Screen minimum width and height (for resizable window)
        Size screenMax;                     // Screen maximum width and height (for resizable window)
        Matrix screenScale;                 // Matrix to scale screen (framebuffer rendering)

        char **dropFilepaths;               // Store dropped files paths pointers (provided by GLFW)
        unsigned int dropFileCount;         // Count dropped files strings

    } Window;
    struct {
        const char *basePath;               // Base path for data storage

    } Storage;
    struct {
        struct {
            int exitKey;                    // Default exit key
            char currentKeyState[MAX_KEYBOARD_KEYS]; // Registers current frame key state
            char previousKeyState[MAX_KEYBOARD_KEYS]; // Registers previous frame key state

            // NOTE: Since key press logic involves comparing previous vs current key state,
            // key repeats needs to be handled specially
            char keyRepeatInFrame[MAX_KEYBOARD_KEYS]; // Registers key repeats for current frame

            int keyPressedQueue[MAX_KEY_PRESSED_QUEUE]; // Input keys queue
            int keyPressedQueueCount;       // Input keys queue count

            int charPressedQueue[MAX_CHAR_PRESSED_QUEUE]; // Input characters queue (unicode)
            int charPressedQueueCount;      // Input characters queue count

        } Keyboard;
        struct {
            Vector2 offset;                 // Mouse offset
            Vector2 scale;                  // Mouse scaling
            Vector2 currentPosition;        // Mouse position on screen
            Vector2 previousPosition;       // Previous mouse position
            Vector2 lockedPosition;         // Mouse position when locked

            int cursor;                     // Tracks current mouse cursor
            bool cursorHidden;              // Track if cursor is hidden
            bool cursorLocked;              // Track if cursor is locked (disabled)
            bool cursorOnScreen;            // Tracks if cursor is inside client area

            char currentButtonState[MAX_MOUSE_BUTTONS]; // Registers current mouse button state
            char previousButtonState[MAX_MOUSE_BUTTONS]; // Registers previous mouse button state
            Vector2 currentWheelMove;       // Registers current mouse wheel variation
            Vector2 previousWheelMove;      // Registers previous mouse wheel variation

        } Mouse;
        struct {
            int pointCount;                                 // Number of touch points active
            int pointId[MAX_TOUCH_POINTS];                  // Point identifiers
            Vector2 position[MAX_TOUCH_POINTS];             // Touch position on screen
            Vector2 previousPosition[MAX_TOUCH_POINTS];     // Previous touch position on screen
            char currentTouchState[MAX_TOUCH_POINTS];       // Registers current touch state
            char previousTouchState[MAX_TOUCH_POINTS];      // Registers previous touch state

        } Touch;
        struct {
            int lastButtonPressed;          // Register last gamepad button pressed
            int axisCount[MAX_GAMEPADS];    // Register number of available gamepad axes
            bool ready[MAX_GAMEPADS];       // Flag to know if gamepad is ready
            char name[MAX_GAMEPADS][MAX_GAMEPAD_NAME_LENGTH];               // Gamepad name holder
            char currentButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state
            char previousButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
            float axisState[MAX_GAMEPADS][MAX_GAMEPAD_AXES];                // Gamepad axes state

        } Gamepad;
    } Input;
    struct {
        double current;                     // Current time measure (seconds)
        double previous;                    // Previous time measure (seconds)
        double update;                      // Time measure for frame update (seconds)
        double draw;                        // Time measure for frame draw (seconds)
        double frame;                       // Time measure for one frame (seconds)
        double target;                      // Desired time for one frame, if 0 not applied (seconds)
        unsigned long long base;            // Base time measure for hi-res timer (ticks or nanoseconds)
        unsigned int frameCounter;          // Frame counter (frames)

    } Time;
} CoreData;

extern CoreData CORE;    // Global CORE state data

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);   // Initialize graphics device

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

// yoinked from rlsw.h
// Clamp an integer.
static inline int clamp_int(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

// Clamp a float.
static inline int clamp_float(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool WindowShouldClose(void)
{
    // If the system is ready, it'll check for the should close.
    // The system becomes no longer ready after CloseWindow is called.  
    // Thus this function just lets the programmer escape their endless loop.
    // We allow this on Pico as the default bootloader has a sleep mode when main returns.
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    // Pico is "always in full screen"
    TRACELOG(LOG_WARNING, "ToggleFullscreen() not available on target platform");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    // Pico is "always in full screen"
    TRACELOG(LOG_WARNING, "ToggleBorderlessWindowed() not available on target platform");
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

// Set window state: minimized
void MinimizeWindow(void)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowState() not available on target platform");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

// Set icon for window
void SetWindowIcon(Image image)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowTitle() not available on target platform");
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    // Pico has no windowing system.
    TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

// This library supports many display targets, including VGA and DVI.
// Since the device is expected to have PSRAM, we can let those outputs adjust their scale.
// LCDs etcetera must cap to their hardware limits instead.

// Overrides using pico_display.c the minimum resolution with a capper, if needed.
extern void GetMinimumResolution(int* width, int* height);

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    // The programmer can set a cap for a "player's options" in here.
    int newWidth = width;
    int newHeight = height;

    GetMinimumResolution(&newWidth, &newHeight);

    // TRACELOG might not be ready by then
    if (width != newWidth && height != newHeight)
    {
        printf("[DEVICE] [WARNING] SetWindowMinSize() was clamped back to (%i, %i)\n", newWidth, newHeight);
    }

    CORE.Window.screenMin.width = newWidth;
    CORE.Window.screenMin.height = newHeight;
}

// Overrides using pico_display.c the maximum resolution with a capper, if needed.
extern void GetMaximumResolution(int* width, int* height);

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    // The programmer can set a cap for a "player's options" in here.
    int newWidth = width;
    int newHeight = height;
    
    GetMaximumResolution(&newWidth, &newHeight);

    // TRACELOG might not be ready by then.
    if (width != newWidth && height != newHeight)
    {
        printf("[DEVICE] [WARNING] SetWindowMaxSize() was clamped back to (%i, %i)\n", newWidth, newHeight);
    }

    CORE.Window.screenMax.width = newWidth;
    CORE.Window.screenMax.height = newHeight;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    int newWidth = width;
    int newHeight = height;

    newWidth = clamp_int(newWidth, CORE.Window.screenMin.width, CORE.Window.screenMax.width);
    newHeight = clamp_int(newHeight, CORE.Window.screenMin.height, CORE.Window.screenMax.height);

    // TRACELOG might not be ready by then.
    if (width != newWidth && height != newHeight)
    {
        printf("[DEVICE] [WARNING] SetWindowSize() was clamped back to (%i, %i)\n", newWidth, newHeight);
    }

    CORE.Window.screen.width = newWidth;
    CORE.Window.screen.height = newHeight;

    // TODO: actually reallocate all three buffers (depth, color, back color) to the new size.  This needs to be done inside of rlsw so the actual pixel arrays get modified.
    //SetupViewport(newWidth, newHeight);
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

// Get native window handle
void *GetWindowHandle(void)
{
    TRACELOG(LOG_WARNING, "GetWindowHandle() not implemented on target platform");
    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    TRACELOG(LOG_WARNING, "GetMonitorCount() not implemented on target platform");
    return 1;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    TRACELOG(LOG_WARNING, "GetCurrentMonitor() not implemented on target platform");
    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    return CORE.Window.screen.width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    return CORE.Window.screen.height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    // No hardware identification communication channels over VGA or SPI.
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    // No hardware identification communication channels over VGA or SPI.
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight() not implemented on target platform");
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    // SPI devices don't refresh in scanlines, and VGA ones cannot be reliably clocked to different values.  So no good purpose here.
    TRACELOG(LOG_WARNING, "GetMonitorRefreshRate() not implemented on target platform");
    return 0;
}

extern const char* GetMonitorDeviceName(void);

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    // No monitor selection implemented.
    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "Monitor index other than 0 is not supported on target platform");
    }
    return GetMonitorDeviceName();
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    TRACELOG(LOG_WARNING, "GetWindowPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    TRACELOG(LOG_WARNING, "GetWindowScaleDPI() not implemented on target platform");
    return (Vector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText() not implemented on target platform");
    return NULL;
}

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };

    TRACELOG(LOG_WARNING, "GetClipboardImage() not implemented on target platform");

    return image;
}

// Show mouse cursor
void ShowCursor(void)
{
    // TODO
}

// Hide mouse cursor
void HideCursor(void)
{
    // TODO
}

// Enable cursor (unlock cursor)
void EnableCursor(void)
{
    // TODO
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = false;
}

// Disable cursor (lock cursor)
void DisableCursor(void)
{
    // TODO
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = true;
}

// Get it by force from rlsw.h at link time.
extern void* swGetColorBuffer(int* width, int* height);
#ifdef SW_DOUBLE_BUFFERING
extern void swSwapColorBuffers();
#endif

// In R5G6B5 format, transmit the buffer via pico_display.c.  Block raylib in here if we're waiting for Core 1 to finish.
extern void FlipBuffer(uint16_t* buffer, int screenWidth, int screenHeight);

#ifdef HAS_VGA_CORE1_STARTUP
// Deferred Core 1 startup for VGA (after rlsw buffers allocated)
extern void VGAStartCore1(void);
#endif

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
#ifdef HAS_VGA_CORE1_STARTUP
    static bool vgaCoreStarted = false;
    if (!vgaCoreStarted) {
        printf("[DEVICE] Calling VGA Start...\n");
        VGAStartCore1();
        vgaCoreStarted = true;
    }
#endif

    int swWidth;
    int swHeight;
    uint16_t* swFramebuffer = (uint16_t*)swGetColorBuffer(&swWidth, &swHeight);
    
    if (!swFramebuffer || swWidth != CORE.Window.screen.width || swHeight != CORE.Window.screen.height)
    {
        TRACELOG(LOG_ERROR, "SwapScreenBuffer() failed: software framebuffer not available or size mismatch");
        return;
    }

#ifdef SW_DOUBLE_BUFFERING
    swSwapColorBuffers();
#endif
    FlipBuffer(swFramebuffer, swWidth, swHeight);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Pico 2 has a hardware timer, let's use it.
#include "hardware/timer.h"

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    // InitTimer calls this, so it doesn't matter what the value of the hardware timer was here.
    // Because that will then be used as the start.

    return (double)time_us_64() / 1000000.0;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // TODO: Research possibilities for opening FTPs etc.

    // Security check to (partially) avoid malicious code on target platform
    //if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    //else
    //{
        // TODO: Load url using default browser
    //}
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings() not implemented on target platform");
    return 0;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration() not implemented on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    TRACELOG(LOG_WARNING, "SetMouseCursor() not implemented on target platform");
}

// Get physical key name.
const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName() not implemented on target platform");
    return "";
}

// Emulate keyboards in pico_display.c
extern void PollInput(void);
extern int numButtonsToTest;

#ifdef RAY_BUTTON_IMPLEMENTATION
extern PicoButton picoButtonTable[];
#endif

// Register all input events
void PollInputEvents(void)
{    
#if SUPPORT_GESTURES_SYSTEM
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is called on an event, not every frame
    UpdateGestures();
#endif

    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;

    // Register previous key state and reset repeat flags
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Reset mouse wheel
    CORE.Input.Mouse.currentWheelMove.x = 0;
    CORE.Input.Mouse.currentWheelMove.y = 0;

    PollInput();

#ifdef RAY_BUTTON_IMPLEMENTATION

    for (int i = 0; i < numButtonsToTest; i++)
    {
        if (picoButtonTable[i].isDown)
        {
            // printf("%d pressed.\n", picoButtons[i]);
            // If key was up, add it to the key pressed queue
            if ((CORE.Input.Keyboard.currentKeyState[picoButtonTable[i].key] == 0) && (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE))
            {
                CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = picoButtonTable[i].key;
                CORE.Input.Keyboard.keyPressedQueueCount++;
            }

            CORE.Input.Keyboard.currentKeyState[picoButtonTable[i].key] = 1;
        }
        else
        {
            CORE.Input.Keyboard.currentKeyState[picoButtonTable[i].key] = 0;
        }
    }
#endif

    if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey]) 
    {
        CORE.Window.shouldClose = true;
    }

}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Setup keyboard emulation via pico_display.c in here.
extern void InitInput(void);

// Initialise display drivers via pico_display.c
extern void InitDisplay(unsigned int width, unsigned int height);

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    // We need to override the core resolutions, so that rcore.c sets the render res correctly before initializing RLSW.
    SetWindowMinSize(0, 0);
    SetWindowMaxSize(1366, 768); // Maximum amount of PSRAM usage.
    SetWindowSize(CORE.Window.screen.width, CORE.Window.screen.height);

    InitDisplay(CORE.Window.screen.width, CORE.Window.screen.height);

    // Ensure render dimensions are set (if not set by platform, use screen dimensions)
    if (CORE.Window.render.width == 0 || CORE.Window.render.height == 0) {
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
    }

    // Initialize currentFbo to the render dimensions for BeginMode3D calculations
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;

    InitInput();

    // Manually InitTimer();
    CORE.Time.previous = GetTime(); // Get time as double

    CORE.Storage.basePath = GetWorkingDirectory();

    CORE.Window.ready = true;

    TRACELOG(LOG_INFO, "PLATFORM: RP2350: Initialized successfully");

    return 0;
}

#include "hardware/xosc.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"

// Clean up the display driver via pico_display.c
extern void CleanupDisplay(void);

// Close platform
void ClosePlatform(void)
{
    CleanupDisplay();

    sleep_run_from_xosc();

    // No way to wake from sleep.
    while (true) 
    {
        tight_loop_contents();
    }
}

// EOF
