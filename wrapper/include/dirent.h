// Clone of https://github.com/georgik/esp-idf-component-raylib/blob/main/raylib/include/dirent.h

/**********************************************************************************************
*
*   dirent.h stub for Pico2 Embedded Platform
*
*   Provides minimal stubs for directory functions not available/needed on Pico2
*   This prevents compilation errors when raylib tries to include <dirent.h>
*
**********************************************************************************************/
#ifndef DIRENT_H_PICO2_STUB
#define DIRENT_H_PICO2_STUB

#include <stddef.h>

// Stub types 
typedef struct DIR DIR;
struct dirent {
    char d_name[256];
};

// TODO: Emulate directory listings, probably by just wrapping lfs directly.
static inline DIR *opendir(const char *name) { return NULL; }
static inline struct dirent *readdir(DIR *dirp) { return NULL; }
static inline int closedir(DIR *dirp) { return -1; }

#endif // DIRENT_H_PICO2_STUB