#include "SolidSyslogPosixProcessId.h"
#include "SolidSyslogFormatter.h"

#include <unistd.h>

void SolidSyslogPosixProcessId_Get(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_Uint32(formatter, (uint32_t) getpid());
}
