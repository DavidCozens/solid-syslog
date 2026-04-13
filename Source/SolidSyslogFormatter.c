#include "SolidSyslogFormatter.h"
#include "SolidSyslogFormat.h"

static size_t CountDigits(uint32_t value);
static void   FormatDigitsReverse(char* buffer, uint32_t value, size_t count);

void SolidSyslogFormatter_Create(struct SolidSyslogFormatter* formatter, char* buffer, size_t size)
{
    formatter->buffer   = buffer;
    formatter->size     = size;
    formatter->position = 0;
}

size_t SolidSyslogFormatter_Character(struct SolidSyslogFormatter* formatter, char value)
{
    formatter->buffer[formatter->position] = value;
    formatter->position++;
    formatter->buffer[formatter->position] = '\0';
    return 1;
}

size_t SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxLength)
{
    size_t len = 0;

    while ((len < maxLength) && (source[len] != '\0'))
    {
        formatter->buffer[formatter->position] = source[len];
        formatter->position++;
        len++;
    }
    formatter->buffer[formatter->position] = '\0';

    return len;
}

size_t SolidSyslogFormatter_Uint32(struct SolidSyslogFormatter* formatter, uint32_t value)
{
    size_t digits = CountDigits(value);

    FormatDigitsReverse(formatter->buffer + formatter->position, value, digits);
    formatter->position += digits;
    formatter->buffer[formatter->position] = '\0';

    return digits;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- value is the number, width is the output field width; distinct semantics
size_t SolidSyslogFormatter_PaddedUint32(struct SolidSyslogFormatter* formatter, uint32_t value, size_t width)
{
    size_t digits  = CountDigits(value);
    size_t padding = (width > digits) ? width - digits : 0;

    for (size_t i = 0; i < padding; i++)
    {
        formatter->buffer[formatter->position] = '0';
        formatter->position++;
    }

    FormatDigitsReverse(formatter->buffer + formatter->position, value, digits);
    formatter->position += digits;
    formatter->buffer[formatter->position] = '\0';

    return padding + digits;
}

char* SolidSyslogFormatter_WritePointer(struct SolidSyslogFormatter* formatter)
{
    return formatter->buffer + formatter->position;
}

size_t SolidSyslogFormatter_Remaining(struct SolidSyslogFormatter* formatter)
{
    return formatter->size - formatter->position;
}

void SolidSyslogFormatter_Advance(struct SolidSyslogFormatter* formatter, size_t count)
{
    formatter->position += count;
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

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- value is the number to format, count is the digit width; distinct semantics
static void FormatDigitsReverse(char* buffer, uint32_t value, size_t count)
{
    size_t i = count;

    while (i > 0)
    {
        i--;
        buffer[i] = SolidSyslogFormat_DigitToChar(value);
        value /= 10U;
    }
}
