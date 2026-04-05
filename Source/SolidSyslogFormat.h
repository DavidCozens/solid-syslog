#ifndef SOLIDSYSLOGFORMAT_H
#define SOLIDSYSLOGFORMAT_H

#include <stddef.h>
#include <stdint.h>

static inline size_t SolidSyslogFormat_MinSize(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

static inline size_t SolidSyslogFormat_Character(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
}

static inline size_t SolidSyslogFormat_Nilvalue(char* buffer)
{
    return SolidSyslogFormat_Character(buffer, '-');
}

static inline char SolidSyslogFormat_DigitToChar(uint32_t value)
{
    return (char) ('0' + (value % 10U));
}

size_t SolidSyslogFormat_BoundedString(char* buffer, const char* source, size_t bufferSize);
size_t SolidSyslogFormat_Uint32(char* buffer, uint32_t value);

#endif /* SOLIDSYSLOGFORMAT_H */
