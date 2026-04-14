#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- test helper macros for readability; CppUTest requires macros for correct failure location

#define CREATE_FORMATTER(bufferSize) formatter = SolidSyslogFormatter_Create(storage, bufferSize)

#define CHECK_FORMATTED(expected) STRCMP_EQUAL(expected, SolidSyslogFormatter_AsString(formatter))

#define CHECK_LENGTH(expected) LONGS_EQUAL(expected, SolidSyslogFormatter_Length(formatter))

// NOLINTEND(cppcoreguidelines-macro-usage)

enum
{
    TEST_BUFFER_SIZE = 64
};

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
};

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    SolidSyslogFormatter_Character(formatter, 'A');

    CHECK_LENGTH(1);
    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    SolidSyslogFormatter_Character(formatter, 'A');
    SolidSyslogFormatter_Character(formatter, 'B');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    CHECK_LENGTH(5);
    CHECK_FORMATTED("hello");
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    SolidSyslogFormatter_BoundedString(formatter, "hello", 3);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(formatter, '<');
    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    CHECK_FORMATTED("<hello");
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    SolidSyslogFormatter_Uint32(formatter, 7);

    CHECK_LENGTH(1);
    CHECK_FORMATTED("7");
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    SolidSyslogFormatter_Uint32(formatter, 0);

    CHECK_FORMATTED("0");
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    SolidSyslogFormatter_Uint32(formatter, 134);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("134");
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(formatter, '<');
    SolidSyslogFormatter_Uint32(formatter, 42);

    CHECK_FORMATTED("<42");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsSingleDigitToWidth2)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 5, 2);

    CHECK_LENGTH(2);
    CHECK_FORMATTED("05");
}

TEST(SolidSyslogFormatter, PaddedUint32NoPaddingWhenValueFillsWidth)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 12, 2);

    CHECK_LENGTH(2);
    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsYearToWidth4)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 2009, 4);

    CHECK_LENGTH(4);
    CHECK_FORMATTED("2009");
}

TEST(SolidSyslogFormatter, PaddedUint32PadsMicrosecondsToWidth6)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 123, 6);

    CHECK_LENGTH(6);
    CHECK_FORMATTED("000123");
}

TEST(SolidSyslogFormatter, PaddedUint32ZeroPadsToFullWidth)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 0, 2);

    CHECK_LENGTH(2);
    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, LengthStartsAtZero)
{
    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, LengthAdvancesWithWrites)
{
    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    CHECK_LENGTH(5);
}

TEST(SolidSyslogFormatter, AsStringReturnsFormattedContent)
{
    SolidSyslogFormatter_BoundedString(formatter, "test", 4);

    CHECK_FORMATTED("test");
}

TEST(SolidSyslogFormatter, ZeroSizeCharacterIsNoOp)
{
    CREATE_FORMATTER(0);

    SolidSyslogFormatter_Character(formatter, 'A');

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeBoundedStringIsNoOp)
{
    CREATE_FORMATTER(0);

    SolidSyslogFormatter_BoundedString(formatter, "hello", 5);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeUint32IsNoOp)
{
    CREATE_FORMATTER(0);

    SolidSyslogFormatter_Uint32(formatter, 42);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, OneByteBufferHoldsOnlyNullTerminator)
{
    CREATE_FORMATTER(1);

    SolidSyslogFormatter_Character(formatter, 'X');

    CHECK_LENGTH(0);
    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, BoundedStringStopsAtBufferCapacity)
{
    CREATE_FORMATTER(4);

    SolidSyslogFormatter_BoundedString(formatter, "abcdef", 10);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringFillsExactCapacity)
{
    CREATE_FORMATTER(4);

    SolidSyslogFormatter_BoundedString(formatter, "abcdef", 6);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringWritesNothingWhenFull)
{
    CREATE_FORMATTER(4);

    SolidSyslogFormatter_BoundedString(formatter, "abc", 3);
    SolidSyslogFormatter_BoundedString(formatter, "xyz", 3);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, CharacterStopsWhenFull)
{
    CREATE_FORMATTER(3);

    SolidSyslogFormatter_Character(formatter, 'A');
    SolidSyslogFormatter_Character(formatter, 'B');
    SolidSyslogFormatter_Character(formatter, 'C');

    CHECK_LENGTH(2);
    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, Uint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    SolidSyslogFormatter_Uint32(formatter, 12345);

    CHECK_LENGTH(2);
    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, Uint32FitsExactly)
{
    CREATE_FORMATTER(4);

    SolidSyslogFormatter_Uint32(formatter, 123);

    CHECK_LENGTH(3);
    CHECK_FORMATTED("123");
}

TEST(SolidSyslogFormatter, PaddedUint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    SolidSyslogFormatter_PaddedUint32(formatter, 5, 4);

    CHECK_LENGTH(2);
    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, PaddedUint32FitsExactly)
{
    CREATE_FORMATTER(5);

    SolidSyslogFormatter_PaddedUint32(formatter, 9, 4);

    CHECK_LENGTH(4);
    CHECK_FORMATTED("0009");
}
