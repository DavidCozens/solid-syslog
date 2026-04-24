#include "SolidSyslogFormatter.h"
#include "SolidSyslogMacros.h"

#include <stdbool.h>
#include <stddef.h>

struct SolidSyslogFormatter
{
    size_t size;
    size_t position;
    char   buffer[];
};

SOLIDSYSLOG_STATIC_ASSERT(sizeof(struct SolidSyslogFormatter) == SOLIDSYSLOG_FORMATTER_OVERHEAD * sizeof(SolidSyslogFormatterStorage),
                          "SOLIDSYSLOG_FORMATTER_OVERHEAD does not match struct layout");

static const char QUOTE                      = '"';
static const char BACKSLASH                  = '\\';
static const char CLOSE_BRACKET              = ']';
static const char ESCAPE_PREFIX              = '\\';
static const char LOWEST_PRINTABLE_US_ASCII  = '!';
static const char HIGHEST_PRINTABLE_US_ASCII = '~';
static const char NON_PRINTABLE_SUBSTITUTE   = '?';

static const char REPLACEMENT_CHARACTER[] = {'\xEF', '\xBF', '\xBD'};

static inline void   WriteChar(struct SolidSyslogFormatter* formatter, char value);
static inline void   WriteEscapedChar(struct SolidSyslogFormatter* formatter, char value);
static inline void   WritePrintableUsAsciiChar(struct SolidSyslogFormatter* formatter, char value);
static inline void   NullTerminate(struct SolidSyslogFormatter* formatter);
static inline bool   NeedsEscape(char value);
static inline bool   IsPrintableUsAscii(char value);
static inline bool   IsValidUtf8SingleByte(char value);
static inline bool   IsValidUtf8TwoByte(char lead, char continuation);
static inline bool   IsValidUtf8ThreeByte(char lead, char continuation1, char continuation2);
static inline bool   IsValidUtf8FourByte(char lead, char continuation1, char continuation2, char continuation3);
static inline size_t Utf8CodepointLength(const char* source);
static inline char   DigitToChar(uint32_t value);
static size_t        CountDigits(uint32_t value);

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
        const char* copyFrom = &source[len];
        size_t      copyCount;

        size_t length = Utf8CodepointLength(&source[len]);
        if (length > 0)
        {
            copyCount = length;
            len += length;
        }
        else
        {
            copyFrom  = REPLACEMENT_CHARACTER;
            copyCount = sizeof(REPLACEMENT_CHARACTER);
            len += 1;
        }

        for (size_t i = 0; i < copyCount; i++)
        {
            WriteChar(formatter, copyFrom[i]);
        }
    }
    NullTerminate(formatter);
}

static inline bool IsValidUtf8SingleByte(char value)
{
    return (value & 0x80) == 0x00;
}

static inline bool IsValidUtf8TwoByte(char lead, char continuation)
{
    return ((lead & 0xE0) == 0xC0) && ((lead & 0xFE) != 0xC0) && ((continuation & 0xC0) == 0x80);
}

static inline bool IsValidUtf8ThreeByte(char lead, char continuation1, char continuation2)
{
    return ((lead & 0xF0) == 0xE0) && ((continuation1 & 0xC0) == 0x80) && ((continuation2 & 0xC0) == 0x80)
           && !((lead == '\xE0') && ((continuation1 & 0xE0) == 0x80))
           && !((lead == '\xED') && ((continuation1 & 0xE0) == 0xA0));
}

static inline bool IsValidUtf8FourByte(char lead, char continuation1, char continuation2, char continuation3)
{
    return (((lead & 0xFC) == 0xF0) || (lead == '\xF4')) && ((continuation1 & 0xC0) == 0x80) && ((continuation2 & 0xC0) == 0x80) && ((continuation3 & 0xC0) == 0x80)
           && !((lead == '\xF0') && ((continuation1 & 0xF0) == 0x80))
           && !((lead == '\xF4') && ((continuation1 & 0xF0) != 0x80));
}

static inline size_t Utf8CodepointLength(const char* source)
{
    if (IsValidUtf8SingleByte(source[0]))
    {
        return 1;
    }
    if (IsValidUtf8TwoByte(source[0], source[1]))
    {
        return 2;
    }
    if (IsValidUtf8ThreeByte(source[0], source[1], source[2]))
    {
        return 3;
    }
    if (IsValidUtf8FourByte(source[0], source[1], source[2], source[3]))
    {
        return 4;
    }
    return 0;
}

void SolidSyslogFormatter_EscapedString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxRawLength)
{
    size_t len = 0;

    while ((len < maxRawLength) && (source[len] != '\0'))
    {
        WriteEscapedChar(formatter, source[len]);
        len++;
    }
    NullTerminate(formatter);
}

static inline void WriteEscapedChar(struct SolidSyslogFormatter* formatter, char value)
{
    if (NeedsEscape(value))
    {
        WriteChar(formatter, ESCAPE_PREFIX);
    }
    WriteChar(formatter, value);
}

static inline bool NeedsEscape(char value)
{
    return (value == QUOTE) || (value == BACKSLASH) || (value == CLOSE_BRACKET);
}

void SolidSyslogFormatter_PrintUsAsciiString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxLength)
{
    size_t len = 0;

    while ((len < maxLength) && (source[len] != '\0'))
    {
        WritePrintableUsAsciiChar(formatter, source[len]);
        len++;
    }
    NullTerminate(formatter);
}

static inline void WritePrintableUsAsciiChar(struct SolidSyslogFormatter* formatter, char value)
{
    if (IsPrintableUsAscii(value))
    {
        WriteChar(formatter, value);
    }
    else
    {
        WriteChar(formatter, NON_PRINTABLE_SUBSTITUTE);
    }
}

static inline bool IsPrintableUsAscii(char value)
{
    return (value >= LOWEST_PRINTABLE_US_ASCII) && (value <= HIGHEST_PRINTABLE_US_ASCII);
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

void SolidSyslogFormatter_TwoDigit(struct SolidSyslogFormatter* formatter, uint32_t value)
{
    WriteChar(formatter, DigitToChar(value / 10U));
    WriteChar(formatter, DigitToChar(value));
    NullTerminate(formatter);
}

void SolidSyslogFormatter_FourDigit(struct SolidSyslogFormatter* formatter, uint32_t value)
{
    WriteChar(formatter, DigitToChar(value / 1000U));
    WriteChar(formatter, DigitToChar(value / 100U));
    WriteChar(formatter, DigitToChar(value / 10U));
    WriteChar(formatter, DigitToChar(value));
    NullTerminate(formatter);
}

void SolidSyslogFormatter_SixDigit(struct SolidSyslogFormatter* formatter, uint32_t value)
{
    WriteChar(formatter, DigitToChar(value / 100000U));
    WriteChar(formatter, DigitToChar(value / 10000U));
    WriteChar(formatter, DigitToChar(value / 1000U));
    WriteChar(formatter, DigitToChar(value / 100U));
    WriteChar(formatter, DigitToChar(value / 10U));
    WriteChar(formatter, DigitToChar(value));
    NullTerminate(formatter);
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
