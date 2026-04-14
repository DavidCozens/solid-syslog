#include "SolidSyslogFormatter.h"

#include <stdbool.h>
#include <stddef.h>

struct SolidSyslogFormatter
{
    size_t size;
    size_t position;
    char   buffer[];
};

_Static_assert(sizeof(struct SolidSyslogFormatter) == SOLIDSYSLOG_FORMATTER_OVERHEAD * sizeof(SolidSyslogFormatterStorage),
               "SOLIDSYSLOG_FORMATTER_OVERHEAD does not match struct layout");

static inline void WriteChar(struct SolidSyslogFormatter* formatter, char value);
static inline void NullTerminate(struct SolidSyslogFormatter* formatter);
static inline char DigitToChar(uint32_t value);
static size_t      CountDigits(uint32_t value);

static inline bool HasCapacity(const struct SolidSyslogFormatter* formatter)
{
    return (formatter->size > 0) && (formatter->position < formatter->size - 1);
}

static inline void WriteChar(struct SolidSyslogFormatter* formatter, char value)
{
    if (HasCapacity(formatter))
    {
        formatter->buffer[formatter->position] = value;
        formatter->position++;
    }
}

static inline void NullTerminate(struct SolidSyslogFormatter* formatter)
{
    if (formatter->size > 0)
    {
        formatter->buffer[formatter->position] = '\0';
    }
}

struct SolidSyslogFormatter* SolidSyslogFormatter_Create(SolidSyslogFormatterStorage* storage, size_t bufferSize)
{
    struct SolidSyslogFormatter* formatter = (struct SolidSyslogFormatter*) storage;
    formatter->size                        = bufferSize;
    formatter->position                    = 0;
    NullTerminate(formatter);
    return formatter;
}

void SolidSyslogFormatter_Character(struct SolidSyslogFormatter* formatter, char value)
{
    WriteChar(formatter, value);
    NullTerminate(formatter);
}

void SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxLength)
{
    size_t len = 0;

    while ((len < maxLength) && (source[len] != '\0'))
    {
        WriteChar(formatter, source[len]);
        len++;
    }
    NullTerminate(formatter);
}

void SolidSyslogFormatter_Uint32(struct SolidSyslogFormatter* formatter, uint32_t value)
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
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- value is the number, width is the output field width; distinct semantics
void SolidSyslogFormatter_PaddedUint32(struct SolidSyslogFormatter* formatter, uint32_t value, size_t width)
{
    size_t digits  = CountDigits(value);
    size_t padding = (width > digits) ? width - digits : 0;

    for (size_t i = 0; i < padding; i++)
    {
        WriteChar(formatter, '0');
    }

    SolidSyslogFormatter_Uint32(formatter, value);
}

const char* SolidSyslogFormatter_AsString(const struct SolidSyslogFormatter* formatter)
{
    return formatter->buffer;
}

size_t SolidSyslogFormatter_Length(const struct SolidSyslogFormatter* formatter)
{
    return formatter->position;
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
