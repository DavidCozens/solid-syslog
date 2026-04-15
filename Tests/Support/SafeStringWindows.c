#include "SafeString.h"

#include <string.h>

void SafeString_Copy(char* dest, size_t destSize, const char* src)
{
    strncpy_s(dest, destSize, src, _TRUNCATE);
}
