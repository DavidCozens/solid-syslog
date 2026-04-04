#include "ExampleAppName.h"

#include <string.h>

static const char* appName;

void ExampleAppName_Set(const char* argv0)
{
    const char* slash = strrchr(argv0, '/');
    appName           = (slash != NULL) ? slash + 1 : argv0;
}

size_t ExampleAppName_Get(char* buffer, size_t size)
{
    size_t len = strlen(appName);
    if (len >= size)
    {
        len = size - 1;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded len; memcpy_s is not portable
    memcpy(buffer, appName, len);
    buffer[len] = '\0';
    return len;
}
