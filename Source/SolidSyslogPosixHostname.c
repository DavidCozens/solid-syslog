#include "SolidSyslogPosixHostname.h"

#include <string.h>
#include <unistd.h>

size_t SolidSyslogPosixHostname_Get(char* buffer, size_t size)
{
    size_t len = 0;

    if (gethostname(buffer, size) == 0)
    {
        buffer[size - 1] = '\0';
        len = strlen(buffer);
    }

    buffer[len] = '\0';
    return len;
}
