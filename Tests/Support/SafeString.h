#ifndef SAFESTRING_H
#define SAFESTRING_H

/* Portable safe string copy: MSVC bans strncpy (C4996) and provides strncpy_s,
   which is not available on other toolchains. CMake selects the platform-specific
   implementation (SafeStringWindows.c or SafeStringStandard.c). */

#include <stddef.h>

void SafeString_Copy(char* dest, size_t destSize, const char* src);

#endif /* SAFESTRING_H */
