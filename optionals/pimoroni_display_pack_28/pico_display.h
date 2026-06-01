// Copy of templates/optionals_display

typedef enum {
    PORTRAIT = 0,
    LANDSCAPE = 1,
    INVERTED_PORTRAIT = 2,
    INVERTED_LANDSCAPE = 3
} Orientation;

// Overrides the minimum resolution with a capper, if needed.
void GetMinimumResolution(int* width, int* height);
// Overrides the maximum resolution with a capper, if needed.
void GetMaximumResolution(int* width, int* height);

int GetMonitorWidth();
int GetMonitorHeight();

void SetMonitorOrientation(Orientation orientation);
Orientation GetMonitorOrientation();

inline const char* GetMonitorDeviceName() { return "Pimoroni Pico Display Pack 2.8\""; }