#include "StringFake.h"

#include <string.h>

static const char* fakeHostname;
static const char* fakeAppName;
static const char* fakeProcId;

static size_t ClampToBufferSize(size_t length, size_t bufferSize);
static int    CopyBounded(char* buffer, size_t size, const char* source);

void StringFake_Reset(void)
{
    fakeHostname = "";
    fakeAppName  = "";
    fakeProcId   = "";
}

void StringFake_SetHostname(const char* hostname)
{
    fakeHostname = hostname;
}

int StringFake_GetHostname(char* buffer, size_t size)
{
    return CopyBounded(buffer, size, fakeHostname);
}

void StringFake_SetAppName(const char* appName)
{
    fakeAppName = appName;
}

int StringFake_GetAppName(char* buffer, size_t size)
{
    return CopyBounded(buffer, size, fakeAppName);
}

void StringFake_SetProcId(const char* procId)
{
    fakeProcId = procId;
}

int StringFake_GetProcId(char* buffer, size_t size)
{
    return CopyBounded(buffer, size, fakeProcId);
}

static int CopyBounded(char* buffer, size_t size, const char* source)
{
    size_t len = ClampToBufferSize(strlen(source), size);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded len
    memcpy(buffer, source, len);
    buffer[len] = '\0';
    return (int) len;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- length and bufferSize are semantically distinct
static size_t ClampToBufferSize(size_t length, size_t bufferSize)
{
    size_t maxLength = bufferSize - 1;
    return (length < maxLength) ? length : maxLength;
}
