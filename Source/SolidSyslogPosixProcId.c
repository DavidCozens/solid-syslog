#include "SolidSyslogPosixProcId.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int SolidSyslogPosixProcId_Get(char* buffer, size_t size)
{
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- snprintf is bounded; snprintf_s is not portable
    return snprintf(buffer, size, "%d", (int) getpid());
}
