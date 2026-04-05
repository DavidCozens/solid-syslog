#include "SolidSyslogFormat.h"

static size_t CountSignificantDigits(uint32_t value);
static void   FormatLeastSignificantDigits(char* buffer, uint32_t value, size_t count);

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

size_t SolidSyslogFormat_Uint32(char* buffer, uint32_t value)
{
    size_t digits = CountSignificantDigits(value);
    FormatLeastSignificantDigits(buffer, value, digits);
    buffer[digits] = '\0';
    return digits;
}

static size_t CountSignificantDigits(uint32_t value)
{
    size_t count = 1;
    while (value >= 10U)
    {
        count++;
        value /= 10U;
    }
    return count;
}

static void FormatLeastSignificantDigits(char* buffer, uint32_t value, size_t count)
{
    size_t i = count;
    while (i > 0)
    {
        i--;
        buffer[i] = SolidSyslogFormat_DigitToChar(value);
        value /= 10U;
    }
}
