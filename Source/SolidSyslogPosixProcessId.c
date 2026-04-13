#include "SolidSyslogPosixProcessId.h"
#include "SolidSyslogFormatter.h"

#include <unistd.h>

size_t SolidSyslogPosixProcessId_Get(char* buffer, size_t size)
{
    struct SolidSyslogFormatter f;
    SolidSyslogFormatter_Create(&f, buffer, size);
    SolidSyslogFormatter_Uint32(&f, (uint32_t) getpid());
    return f.position;
}
