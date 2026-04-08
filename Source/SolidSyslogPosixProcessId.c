#include "SolidSyslogPosixProcessId.h"
#include "SolidSyslogFormat.h"

#include <unistd.h>

size_t SolidSyslogPosixProcessId_Get(char* buffer, size_t size)
{
    (void) size;
    return SolidSyslogFormat_Uint32(buffer, (uint32_t) getpid());
}
