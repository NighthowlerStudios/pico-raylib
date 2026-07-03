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

#ifdef SD_CARD
#include <sys/stat.h>
#endif

// Stub defines.
#define	F_OK	0

// Stub types
typedef uint32_t mode_t;

// Stub functions - return NULL/error to indicate not supported
// TODO: Emulate working directory.
inline char* getcwd(char* __buf, size_t __size) { return "/"; }
inline int chdir(const char* __path) { return -1; }

extern void _exit(int status);

#ifndef SD_CARD
//inline int mkdir(const char* _path, mode_t __mode ) { return -1; }
inline int access(const char* __path, int __amode) { return -1;}
#else
// We have the ability to test for file existence, but not using access().  Let's emulate access.
static inline int access(const char *pathname, int mode) { struct stat buffer; return stat(pathname, &buffer);}
#endif

#endif // DIRENT_H_PICO2_STUB