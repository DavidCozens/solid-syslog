#include "StringFake.h"

#include <string.h>

static const char* fakeHostname;

static size_t ClampToBufferSize(size_t length, size_t bufferSize);
static int    CopyBounded(char* buffer, size_t size, const char* source);

void StringFake_Reset(void)
{
    fakeHostname = "";
}

void StringFake_SetHostname(const char* hostname)
{
    fakeHostname = hostname;
}

int StringFake_GetHostname(char* buffer, size_t size)
{
    return CopyBounded(buffer, size, fakeHostname);
}

static int CopyBounded(char* buffer, size_t size, const char* source)
{
    size_t len = ClampToBufferSize(strlen(source), size);
    memcpy(buffer, source, len);
    buffer[len] = '\0';
    return (int) len;
}

static size_t ClampToBufferSize(size_t length, size_t bufferSize)
{
    size_t maxLength = bufferSize - 1;
    return (length < maxLength) ? length : maxLength;
}
