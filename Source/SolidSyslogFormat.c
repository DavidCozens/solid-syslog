#include "SolidSyslogFormat.h"

size_t SolidSyslogFormat_Character(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
}

size_t SolidSyslogFormat_BoundedString(char* buffer, const char* source, size_t bufferSize)
{
    size_t maxLength = (bufferSize > 0) ? bufferSize - 1 : 0;
    size_t len       = 0;
    while ((len < maxLength) && (source[len] != '\0'))
    {
        buffer[len] = source[len];
        len++;
    }
    buffer[len] = '\0';
    return len;
}

size_t SolidSyslogFormat_Nilvalue(char* buffer)
{
    return SolidSyslogFormat_Character(buffer, '-');
}

size_t SolidSyslogFormat_Uint32(char* buffer, uint32_t value)
{
    char   temp[11];
    size_t pos = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    uint32_t remaining = value;
    while (remaining > 0)
    {
        temp[pos++] = (char) ('0' + (remaining % 10));
        remaining /= 10;
    }

    for (size_t i = 0; i < pos; i++)
    {
        buffer[i] = temp[pos - 1 - i];
    }
    buffer[pos] = '\0';
    return pos;
}
