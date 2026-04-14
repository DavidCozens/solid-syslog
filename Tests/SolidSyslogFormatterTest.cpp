#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- test helper macros for readability; CppUTest requires macros for correct failure location

#define CREATE_FORMATTER(bufferSize) formatter = SolidSyslogFormatter_Create(storage, bufferSize)

#define CHECK_FORMATTED(expected)                                              \
    STRCMP_EQUAL(expected, SolidSyslogFormatter_AsString(formatter));            \
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

    void FormatCharacter(char value) { SolidSyslogFormatter_Character(formatter, value); }
    void FormatBoundedString(const char* source, size_t maxLength) { SolidSyslogFormatter_BoundedString(formatter, source, maxLength); }
    void FormatUint32(uint32_t value) { SolidSyslogFormatter_Uint32(formatter, value); }
    void FormatPaddedUint32(uint32_t value, size_t width) { SolidSyslogFormatter_PaddedUint32(formatter, value, width); }
};
// clang-format on

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    FormatCharacter('A');

    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    FormatCharacter('A');
    FormatCharacter('B');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    FormatBoundedString("hello", 10);

    CHECK_FORMATTED("hello");
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    FormatBoundedString("hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    FormatCharacter('<');
    FormatBoundedString("hello", 10);

    CHECK_FORMATTED("<hello");
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    FormatUint32(7);

    CHECK_FORMATTED("7");
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    FormatUint32(0);

    CHECK_FORMATTED("0");
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    FormatUint32(134);

    CHECK_FORMATTED("134");
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    FormatCharacter('<');
    FormatUint32(42);

    CHECK_FORMATTED("<42");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsSingleDigitToWidth2)
{
    FormatPaddedUint32(5, 2);

    CHECK_FORMATTED("05");
}

TEST(SolidSyslogFormatter, PaddedUint32NoPaddingWhenValueFillsWidth)
{
    FormatPaddedUint32(12, 2);

    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsYearToWidth4)
{
    FormatPaddedUint32(2009, 4);

    CHECK_FORMATTED("2009");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsMicrosecondsToWidth6)
{
    FormatPaddedUint32(123, 6);

    CHECK_FORMATTED("000123");
}

TEST(SolidSyslogFormatter, PaddedUint32ZeroPadsToFullWidth)
{
    FormatPaddedUint32(0, 2);

    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, LengthStartsAtZero)
{
    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, LengthAdvancesWithWrites)
{
    FormatBoundedString("hello", 10);

    CHECK_LENGTH(5);
}

TEST(SolidSyslogFormatter, AsStringReturnsFormattedContent)
{
    FormatBoundedString("test", 4);

    CHECK_FORMATTED("test");
}

TEST(SolidSyslogFormatter, ZeroSizeCharacterIsNoOp)
{
    CREATE_FORMATTER(0);

    FormatCharacter('A');

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeBoundedStringIsNoOp)
{
    CREATE_FORMATTER(0);

    FormatBoundedString("hello", 5);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeUint32IsNoOp)
{
    CREATE_FORMATTER(0);

    FormatUint32(42);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, OneByteBufferHoldsOnlyNullTerminator)
{
    CREATE_FORMATTER(1);

    FormatCharacter('X');

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, BoundedStringStopsAtBufferCapacity)
{
    CREATE_FORMATTER(4);

    FormatBoundedString("abcdef", 10);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringFillsExactCapacity)
{
    CREATE_FORMATTER(4);

    FormatBoundedString("abcdef", 6);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringWritesNothingWhenFull)
{
    CREATE_FORMATTER(4);

    FormatBoundedString("abc", 3);
    FormatBoundedString("xyz", 3);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, CharacterStopsWhenFull)
{
    CREATE_FORMATTER(3);

    FormatCharacter('A');
    FormatCharacter('B');
    FormatCharacter('C');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, Uint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    FormatUint32(12345);

    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, Uint32FitsExactly)
{
    CREATE_FORMATTER(4);

    FormatUint32(123);

    CHECK_FORMATTED("123");
}

TEST(SolidSyslogFormatter, PaddedUint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    FormatPaddedUint32(5, 4);

    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, PaddedUint32FitsExactly)
{
    CREATE_FORMATTER(5);

    FormatPaddedUint32(9, 4);

    CHECK_FORMATTED("0009");
}
