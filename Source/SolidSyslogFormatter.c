#include "SolidSyslogFormatter.h"

static inline void WriteChar(struct SolidSyslogFormatter* formatter, char value);
static inline void NullTerminate(struct SolidSyslogFormatter* formatter);
static inline char DigitToChar(uint32_t value);
static size_t      CountDigits(uint32_t value);

static inline void WriteChar(struct SolidSyslogFormatter* formatter, char value)
{
    formatter->buffer[formatter->position] = value;
    formatter->position++;
}

static inline void NullTerminate(struct SolidSyslogFormatter* formatter)
{
    formatter->buffer[formatter->position] = '\0';
}

void SolidSyslogFormatter_Create(struct SolidSyslogFormatter* formatter, char* buffer, size_t size)
{
    formatter->buffer   = buffer;
    formatter->size     = size;
    formatter->position = 0;
}

size_t SolidSyslogFormatter_Character(struct SolidSyslogFormatter* formatter, char value)
{
    WriteChar(formatter, value);
    NullTerminate(formatter);
    return 1;
}

size_t SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxLength)
{
    size_t len = 0;

    while ((len < maxLength) && (source[len] != '\0'))
    {
        WriteChar(formatter, source[len]);
        len++;
    }
    NullTerminate(formatter);

    return len;
}

size_t SolidSyslogFormatter_Uint32(struct SolidSyslogFormatter* formatter, uint32_t value)
{
    size_t   digits  = CountDigits(value);
    uint32_t divisor = 1;

    for (size_t i = 1; i < digits; i++)
    {
        divisor *= 10U;
    }

    for (size_t i = 0; i < digits; i++)
    {
        WriteChar(formatter, DigitToChar(value / divisor));
        value %= divisor;
        divisor /= 10U;
    }
    NullTerminate(formatter);

    return digits;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- value is the number, width is the output field width; distinct semantics
size_t SolidSyslogFormatter_PaddedUint32(struct SolidSyslogFormatter* formatter, uint32_t value, size_t width)
{
    size_t digits  = CountDigits(value);
    size_t padding = (width > digits) ? width - digits : 0;

    for (size_t i = 0; i < padding; i++)
    {
        WriteChar(formatter, '0');
    }

    SolidSyslogFormatter_Uint32(formatter, value);

    return padding + digits;
}

size_t SolidSyslogFormatter_Remaining(struct SolidSyslogFormatter* formatter)
{
    return formatter->size - formatter->position;
}

static inline char DigitToChar(uint32_t value)
{
    return (char) ('0' + (value % 10U));
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
