#ifndef ORIENTATION_H
#define ORIENTATION_H
// Standardise some orientation stuff.

typedef enum Orientation {
    LANDSCAPE = 0,
    PORTRAIT = 1,
    INVERTED_LANDSCAPE = 2,
    INVERTED_PORTRAIT = 3
} Orientation;

// NOTE: Orientation remains consistent throughout runtime.  Change it at compile time instead.  Prevents buffer fragmentation.
extern Orientation currentOrientation;

#endif