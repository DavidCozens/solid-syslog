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
