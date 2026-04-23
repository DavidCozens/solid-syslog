#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- test helper macros for readability; CppUTest requires macros for correct failure location

#define CREATE_FORMATTER(bufferSize) formatter = SolidSyslogFormatter_Create(storage, bufferSize)

#define CHECK_FORMATTED(expected)                                     \
    STRCMP_EQUAL(expected, SolidSyslogFormatter_AsString(formatter)); \
    LONGS_EQUAL(strlen(expected), SolidSyslogFormatter_Length(formatter))

#define CHECK_LENGTH(expected) LONGS_EQUAL(expected, SolidSyslogFormatter_Length(formatter))

// NOLINTEND(cppcoreguidelines-macro-usage)

enum
{
    TEST_BUFFER_SIZE = 64
};

// clang-format off
TEST_GROUP(SolidSyslogFormatter)
{
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(TEST_BUFFER_SIZE)];
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    struct SolidSyslogFormatter* formatter;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- formatter is used across TEST() bodies via CppUTest macro
        CREATE_FORMATTER(TEST_BUFFER_SIZE);
    }

    void formatCharacter(char value) const { SolidSyslogFormatter_Character(formatter, value); }
    void formatBoundedString(const char* source, size_t maxLength) const { SolidSyslogFormatter_BoundedString(formatter, source, maxLength); }
    void formatUint32(uint32_t value) const { SolidSyslogFormatter_Uint32(formatter, value); }
    void formatTwoDigit(uint32_t value) const { SolidSyslogFormatter_TwoDigit(formatter, value); }
    void formatFourDigit(uint32_t value) const { SolidSyslogFormatter_FourDigit(formatter, value); }
    void formatSixDigit(uint32_t value) const { SolidSyslogFormatter_SixDigit(formatter, value); }
};

// clang-format on

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    formatCharacter('A');

    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    formatCharacter('A');
    formatCharacter('B');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    formatBoundedString("hello", 10);

    CHECK_FORMATTED("hello");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadByteWithReplacementChar)
{
    formatBoundedString("\x85", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesSmallestContinuationByte)
{
    formatBoundedString("\x80", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongTwoByteEncodingPerByte)
{
    /* \xC1\x81 — overlong 2-byte form of U+0041. Per RFC 3629 §10 and Unicode
     * §3.9, each invalid byte becomes its own U+FFFD substitution. */
    formatBoundedString("\xC1\x81", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongLeadC0)
{
    /* \xC0 — the other overlong 2-byte lead forbidden by RFC 3629 §4. */
    formatBoundedString("\xC0", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadsF5ToFF)
{
    /* F5-F7: would encode codepoint > U+10FFFF (outside Unicode range).
     * F8-FF: 5+ byte prefix patterns, removed by RFC 3629.
     * \xF5 and \xFF exercise both ends of the range. */
    formatBoundedString("\xF5\xFF", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadsInF8ToFFMid)
{
    /* Interior of the 5+ byte prefix range — drives a mask that covers F8-FF,
     * rather than enumerating each value. */
    formatBoundedString("\xF8\xFE", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    formatBoundedString("hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    formatCharacter('<');
    formatBoundedString("hello", 10);

    CHECK_FORMATTED("<hello");
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    formatUint32(7);

    CHECK_FORMATTED("7");
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    formatUint32(0);

    CHECK_FORMATTED("0");
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    formatUint32(134);

    CHECK_FORMATTED("134");
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    formatCharacter('<');
    formatUint32(42);

    CHECK_FORMATTED("<42");
}

TEST(SolidSyslogFormatter, LengthStartsAtZero)
{
    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, LengthAdvancesWithWrites)
{
    formatBoundedString("hello", 10);

    CHECK_LENGTH(5);
}

TEST(SolidSyslogFormatter, AsStringReturnsFormattedContent)
{
    formatBoundedString("test", 4);

    CHECK_FORMATTED("test");
}

TEST(SolidSyslogFormatter, ZeroSizeCharacterIsNoOp)
{
    CREATE_FORMATTER(0);

    formatCharacter('A');

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeBoundedStringIsNoOp)
{
    CREATE_FORMATTER(0);

    formatBoundedString("hello", 5);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeUint32IsNoOp)
{
    CREATE_FORMATTER(0);

    formatUint32(42);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, OneByteBufferHoldsOnlyNullTerminator)
{
    CREATE_FORMATTER(1);

    formatCharacter('X');

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsAllDigits)
{
    formatTwoDigit(59);

    CHECK_FORMATTED("59");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsZero)
{
    formatTwoDigit(0);

    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsMax)
{
    formatTwoDigit(99);

    CHECK_FORMATTED("99");
}

TEST(SolidSyslogFormatter, TwoDigitBeyondMaxFormatsLeastSignificant)
{
    formatTwoDigit(123);

    CHECK_FORMATTED("23");
}

TEST(SolidSyslogFormatter, FourDigitFormatsAllDigits)
{
    formatFourDigit(2009);

    CHECK_FORMATTED("2009");
}

TEST(SolidSyslogFormatter, FourDigitFormatsZero)
{
    formatFourDigit(0);

    CHECK_FORMATTED("0000");
}

TEST(SolidSyslogFormatter, FourDigitFormatsMax)
{
    formatFourDigit(9999);

    CHECK_FORMATTED("9999");
}

TEST(SolidSyslogFormatter, FourDigitBeyondMaxFormatsLeastSignificant)
{
    formatFourDigit(12345);

    CHECK_FORMATTED("2345");
}

TEST(SolidSyslogFormatter, SixDigitFormatsAllDigits)
{
    formatSixDigit(123456);

    CHECK_FORMATTED("123456");
}

TEST(SolidSyslogFormatter, SixDigitFormatsZero)
{
    formatSixDigit(0);

    CHECK_FORMATTED("000000");
}

TEST(SolidSyslogFormatter, SixDigitFormatsMax)
{
    formatSixDigit(999999);

    CHECK_FORMATTED("999999");
}

TEST(SolidSyslogFormatter, SixDigitBeyondMaxFormatsLeastSignificant)
{
    formatSixDigit(1234567);

    CHECK_FORMATTED("234567");
}

TEST(SolidSyslogFormatter, BoundedStringStopsAtBufferCapacity)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abcdef", 10);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringFillsExactCapacity)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abc", 3);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringWritesNothingWhenFull)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abc", 3);
    formatBoundedString("xyz", 3);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, CharacterStopsWhenFull)
{
    CREATE_FORMATTER(3);

    formatCharacter('A');
    formatCharacter('B');
    formatCharacter('C');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, Uint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    formatUint32(12345);

    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, Uint32FitsExactly)
{
    CREATE_FORMATTER(4);

    formatUint32(123);

    CHECK_FORMATTED("123");
}

TEST(SolidSyslogFormatter, EscapedStringWithEmptyInputWritesNothing)
{
    SolidSyslogFormatter_EscapedString(formatter, "", 0);

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, EscapedStringPassesOrdinaryCharacterThrough)
{
    SolidSyslogFormatter_EscapedString(formatter, "a", 1);

    CHECK_FORMATTED("a");
}

TEST(SolidSyslogFormatter, EscapedStringTruncatesAtMaxRawLength)
{
    SolidSyslogFormatter_EscapedString(formatter, "hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesDoubleQuote)
{
    SolidSyslogFormatter_EscapedString(formatter, "a\"b", 3);

    CHECK_FORMATTED("a\\\"b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesBackslash)
{
    SolidSyslogFormatter_EscapedString(formatter, "a\\b", 3);

    CHECK_FORMATTED("a\\\\b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesCloseBracket)
{
    SolidSyslogFormatter_EscapedString(formatter, "a]b", 3);

    CHECK_FORMATTED("a\\]b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesAllThreeSpecialsInOneValue)
{
    SolidSyslogFormatter_EscapedString(formatter, "\"\\]", 3);

    CHECK_FORMATTED("\\\"\\\\\\]");
}

TEST(SolidSyslogFormatter, EscapedStringMaxRawLengthBoundsInputNotOutput)
{
    SolidSyslogFormatter_EscapedString(formatter, R"(""""")", 2);

    CHECK_FORMATTED(R"(\"\")");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringWithEmptyInputWritesNothing)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "", 0);

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringPassesPrintableCharacterThrough)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "A", 1);

    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringTruncatesAtMaxLength)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesSpace)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "a b", 3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesControlCharacter)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\x01"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesDel)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\x7F"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesHighBitByte)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\xC3"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringPassesBangAndTildeBoundariesThrough)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "!~", 2);

    CHECK_FORMATTED("!~");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringTruncationBoundsSubstitution)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "abc\x01"
                                            "def",
                                            3);

    CHECK_FORMATTED("abc");
}
