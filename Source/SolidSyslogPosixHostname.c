#include "SolidSyslogPosixHostname.h"

#include <string.h>
#include <unistd.h>

int SolidSyslogPosixHostname_Get(char* buffer, size_t size)
{
    if (gethostname(buffer, size) != 0)
    {
        buffer[0] = '\0';
        return 0;
    }

    buffer[size - 1] = '\0';
    return (int) strlen(buffer);
}
