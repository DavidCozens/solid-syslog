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

/* UTF-8 replacement character U+FFFD, emitted in place of each invalid
 * byte per Unicode §3.9 best practice for per-byte maximal subpart. */
static const char REPLACEMENT_CHARACTER[] = {'\xEF', '\xBF', '\xBD'};

static inline bool   HasCapacity(const struct SolidSyslogFormatter* formatter);
static inline bool   HasCapacityFor(const struct SolidSyslogFormatter* formatter, size_t count);
static inline bool   IsAboveUnicodeMaxEncoding(char lead, char continuation1);
static inline bool   IsFourByteLead(char byte);
static inline bool   IsOverlongFourByteEncoding(char lead, char continuation1);
static inline bool   IsOverlongThreeByteEncoding(char lead, char continuation1);
static inline bool   IsOverlongTwoByteLead(char byte);
static inline bool   IsPrintableUsAscii(char value);
static inline bool   IsThreeByteLead(char byte);
static inline bool   IsTwoByteLead(char byte);
static inline bool   IsUtf16SurrogateEncoding(char lead, char continuation1);
static inline bool   IsUtf8Continuation(char byte);
static inline bool   IsValidUtf8FourByte(char lead, char continuation1, char continuation2, char continuation3);
static inline bool   IsValidUtf8SingleByte(char byte);
static inline bool   IsValidUtf8ThreeByte(char lead, char continuation1, char continuation2);
static inline bool   IsValidUtf8TwoByte(char lead, char continuation1);
static inline bool   NeedsEscape(char value);
static inline char   DigitToChar(uint32_t value);
static size_t        CountDigits(uint32_t value);
static inline size_t Utf8CodepointLength(const char* source);
static inline void   NullTerminate(struct SolidSyslogFormatter* formatter);
static inline void   WriteBytes(struct SolidSyslogFormatter* formatter, const char* bytes, size_t count);
static inline void   WriteReplacementCharacter(struct SolidSyslogFormatter* formatter);
static inline void   WriteChar(struct SolidSyslogFormatter* formatter, char value);
static inline void   WriteEscapedChar(struct SolidSyslogFormatter* formatter, char value);
static inline void   WritePrintableUsAsciiChar(struct SolidSyslogFormatter* formatter, char value);

struct SolidSyslogFormatter* SolidSyslogFormatter_Create(SolidSyslogFormatterStorage* storage, size_t bufferSize)
{
    struct SolidSyslogFormatter* formatter = (struct SolidSyslogFormatter*) storage;
    formatter->size                        = bufferSize;
    formatter->position                    = 0;
    NullTerminate(formatter);
    return formatter;
}

static inline void NullTerminate(struct SolidSyslogFormatter* formatter)
{
    if (formatter->size > 0)
    {
        formatter->buffer[formatter->position] = '\0';
    }
}

void SolidSyslogFormatter_Character(struct SolidSyslogFormatter* formatter, char value)
{
    WriteChar(formatter, value);
    NullTerminate(formatter);
}

static inline void WriteChar(struct SolidSyslogFormatter* formatter, char value)
{
    if (HasCapacity(formatter))
    {
        formatter->buffer[formatter->position] = value;
        formatter->position++;
    }
}

static inline bool HasCapacity(const struct SolidSyslogFormatter* formatter)
{
    return (formatter->size > 0) && (formatter->position < formatter->size - 1);
}

/*
 * UTF-8 validation per RFC 3629 §4 and Unicode §3.9.
 *
 *   1-byte  0xxxxxxx                                   U+0000  - U+007F
 *   2-byte  110xxxxx 10xxxxxx                          U+0080  - U+07FF    lead C2-DF
 *   3-byte  1110xxxx 10xxxxxx 10xxxxxx                 U+0800  - U+FFFF    lead E0-EF
 *   4-byte  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx        U+10000 - U+10FFFF  lead F0-F4
 *
 * Excluded sequences:
 *   C0, C1             overlong 2-byte encodings
 *   E0 + cont1 80-9F   overlong 3-byte encodings
 *   ED + cont1 A0-BF   UTF-16 surrogate range (U+D800..U+DFFF)
 *   F0 + cont1 80-8F   overlong 4-byte encodings
 *   F4 + cont1 90-BF   codepoints above U+10FFFF
 *   F5-F7              codepoints above U+10FFFF
 *   F8-FF              5+ byte prefix patterns removed by RFC 3629
 */

void SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter* formatter, const char* source, size_t maxLength)
{
    size_t len = 0;

    while ((len < maxLength) && (source[len] != '\0'))
    {
        size_t codepointLength = Utf8CodepointLength(&source[len]);

        if ((codepointLength > 0) && (codepointLength <= (maxLength - len)))
        {
            if (!HasCapacityFor(formatter, codepointLength))
            {
                break;
            }
            WriteBytes(formatter, &source[len], codepointLength);
            len += codepointLength;
        }
        else
        {
            if (!HasCapacityFor(formatter, sizeof(REPLACEMENT_CHARACTER)))
            {
                break;
            }
            WriteReplacementCharacter(formatter);
            len += 1;
        }
    }
    NullTerminate(formatter);
}

static inline size_t Utf8CodepointLength(const char* source)
{
    size_t length = 0;

    if (IsValidUtf8SingleByte(source[0]))
    {
        length = 1;
    }
    else if (IsValidUtf8TwoByte(source[0], source[1]))
    {
        length = 2;
    }
    else if (IsValidUtf8ThreeByte(source[0], source[1], source[2]))
    {
        length = 3;
    }
    else if (IsValidUtf8FourByte(source[0], source[1], source[2], source[3]))
    {
        length = 4;
    }

    return length;
}

static inline bool IsValidUtf8SingleByte(char byte)
{
    return (byte & 0x80) == 0;
}

static inline bool IsValidUtf8TwoByte(char lead, char continuation1)
{
    return IsTwoByteLead(lead) && !IsOverlongTwoByteLead(lead) && IsUtf8Continuation(continuation1);
}

static inline bool IsTwoByteLead(char byte)
{
    return (byte & 0xE0) == 0xC0;
}

static inline bool IsOverlongTwoByteLead(char byte)
{
    return (byte & 0xFE) == 0xC0;
}

static inline bool IsUtf8Continuation(char byte)
{
    return (byte & 0xC0) == 0x80;
}

static inline bool IsValidUtf8ThreeByte(char lead, char continuation1, char continuation2)
{
    return IsThreeByteLead(lead) && IsUtf8Continuation(continuation1) && IsUtf8Continuation(continuation2) &&
           !IsOverlongThreeByteEncoding(lead, continuation1) && !IsUtf16SurrogateEncoding(lead, continuation1);
}

static inline bool IsThreeByteLead(char byte)
{
    return (byte & 0xF0) == 0xE0;
}

static inline bool IsOverlongThreeByteEncoding(char lead, char continuation1)
{
    return (lead == '\xE0') && ((continuation1 & 0xE0) == 0x80);
}

static inline bool IsUtf16SurrogateEncoding(char lead, char continuation1)
{
    return (lead == '\xED') && ((continuation1 & 0xE0) == 0xA0);
}

static inline bool IsValidUtf8FourByte(char lead, char continuation1, char continuation2, char continuation3)
{
    return IsFourByteLead(lead) && IsUtf8Continuation(continuation1) && IsUtf8Continuation(continuation2) && IsUtf8Continuation(continuation3) &&
           !IsOverlongFourByteEncoding(lead, continuation1) && !IsAboveUnicodeMaxEncoding(lead, continuation1);
}

static inline bool IsFourByteLead(char byte)
{
    return (byte & 0xF8) == 0xF0;
}

static inline bool IsOverlongFourByteEncoding(char lead, char continuation1)
{
    return (lead == '\xF0') && ((continuation1 & 0xF0) == 0x80);
}

static inline bool IsAboveUnicodeMaxEncoding(char lead, char continuation1)
{
    bool f4WithCont1Above8F = (lead == '\xF4') && ((continuation1 & 0xF0) != 0x80);
    bool f5OrHigherLead     = (lead == '\xF5') || (lead == '\xF6') || (lead == '\xF7');
    return f4WithCont1Above8F || f5OrHigherLead;
}

static inline bool HasCapacityFor(const struct SolidSyslogFormatter* formatter, size_t count)
{
    return (formatter->size > 0) && ((formatter->position + count) <= (formatter->size - 1));
}

static inline void WriteReplacementCharacter(struct SolidSyslogFormatter* formatter)
{
    WriteBytes(formatter, REPLACEMENT_CHARACTER, sizeof(REPLACEMENT_CHARACTER));
}

static inline void WriteBytes(struct SolidSyslogFormatter* formatter, const char* bytes, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        WriteChar(formatter, bytes[i]);
    }
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

static inline char DigitToChar(uint32_t value)
{
    return (char) ('0' + (value % 10U));
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
