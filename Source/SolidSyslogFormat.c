#include "SolidSyslogFormat.h"

static size_t   CountDigits(uint32_t value);
static uint32_t PowerOf10(size_t exponent);

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
    size_t   digits  = CountDigits(value);
    uint32_t divisor = PowerOf10(digits - 1);

    for (size_t i = 0; i < digits; i++)
    {
        buffer[i] = (char) ('0' + ((value / divisor) % 10U));
        divisor /= 10U;
    }
    buffer[digits] = '\0';
    return digits;
}

static size_t CountDigits(uint32_t value)
{
    size_t count = 1;
    while (value >= 10U)
    {
        count++;
        value /= 10U;
    }
    return count;
}

static uint32_t PowerOf10(size_t exponent)
{
    uint32_t result = 1;
    for (size_t i = 0; i < exponent; i++)
    {
        result *= 10U;
    }
    return result;
}
