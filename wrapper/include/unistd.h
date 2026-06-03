/**********************************************************************************************
*
*   unistd.h stub for Pico2 Embedded Platform
*
*   Provides minimal stubs for directory functions not available/needed on Pico2
*   This prevents compilation errors when raylib tries to include <unistd.h>
*
**********************************************************************************************/
#ifndef UNISTD_H_PICO2_STUB
#define UNISTD_H_PICO2_STUB

#include <stddef.h>
#include <stdint.h>

// Stub defines.
#define	F_OK	0

// Stub types
typedef uint32_t mode_t;

// Stub functions - return NULL/error to indicate not supported
#ifndef SD_CARD
extern void _exit(int status);
inline char* getcwd(char* __buf, size_t __size) { return "/"; }
inline int chdir(const char* __path) { return -1; }
//inline int mkdir(const char* _path, mode_t __mode ) { return -1; }
inline int access(const char* __path, int __amode) { return -1;}
#else
// TODO: implement SD card library as a file system
#endif

#endif // DIRENT_H_PICO2_STUB