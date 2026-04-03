#include "SolidSyslogPosixProcId.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

size_t SolidSyslogPosixProcId_Get(char* buffer, size_t size)
{
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- snprintf is bounded; snprintf_s is not portable
    int written = snprintf(buffer, size, "%d", (int) getpid());
    return (written > 0) ? (size_t) written : 0;
}
