#ifndef SOLIDSYSLOGFORMAT_H
#define SOLIDSYSLOGFORMAT_H

#include <stddef.h>
#include <stdint.h>

static inline size_t CountSignificantDigits(uint32_t value);
static inline void   FormatLeastSignificantDigits(char* buffer, uint32_t value, size_t count);

static inline size_t SolidSyslogFormat_Character(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
}

static inline size_t SolidSyslogFormat_BoundedString(char* buffer, const char* source, size_t bufferSize)
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

static inline size_t SolidSyslogFormat_Nilvalue(char* buffer)
{
    return SolidSyslogFormat_Character(buffer, '-');
}

static inline size_t SolidSyslogFormat_Uint32(char* buffer, uint32_t value)
{
    size_t digits = CountSignificantDigits(value);
    FormatLeastSignificantDigits(buffer, value, digits);
    buffer[digits] = '\0';
    return digits;
}

static inline size_t CountSignificantDigits(uint32_t value)
{
    size_t count = 1;
    while (value >= 10U)
    {
        count++;
        value /= 10U;
    }
    return count;
}

static inline void FormatLeastSignificantDigits(char* buffer, uint32_t value, size_t count)
{
    size_t i = count;
    while (i > 0)
    {
        i--;
        buffer[i] = (char) ('0' + (value % 10U));
        value /= 10U;
    }
}

#endif /* SOLIDSYSLOGFORMAT_H */
