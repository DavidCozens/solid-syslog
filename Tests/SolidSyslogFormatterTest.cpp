#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

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
        formatter = SolidSyslogFormatter_Create(storage, TEST_BUFFER_SIZE);
    }
};

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    size_t written = SolidSyslogFormatter_Character(formatter, 'A');

    LONGS_EQUAL(1, written);
    STRCMP_EQUAL("A", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    SolidSyslogFormatter_Character(formatter, 'A');
    SolidSyslogFormatter_Character(formatter, 'B');

    STRCMP_EQUAL("AB", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    size_t written = SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    LONGS_EQUAL(5, written);
    STRCMP_EQUAL("hello", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    size_t written = SolidSyslogFormatter_BoundedString(formatter, "hello", 3);

    LONGS_EQUAL(3, written);
    STRCMP_EQUAL("hel", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(formatter, '<');
    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    STRCMP_EQUAL("<hello", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    size_t written = SolidSyslogFormatter_Uint32(formatter, 7);

    LONGS_EQUAL(1, written);
    STRCMP_EQUAL("7", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    SolidSyslogFormatter_Uint32(formatter, 0);

    STRCMP_EQUAL("0", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    size_t written = SolidSyslogFormatter_Uint32(formatter, 134);

    LONGS_EQUAL(3, written);
    STRCMP_EQUAL("134", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(formatter, '<');
    SolidSyslogFormatter_Uint32(formatter, 42);

    STRCMP_EQUAL("<42", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32PadsSingleDigitToWidth2)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(formatter, 5, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("05", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32NoPaddingWhenValueFillsWidth)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(formatter, 12, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("12", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32PadsYearToWidth4)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(formatter, 2009, 4);

    LONGS_EQUAL(4, written);
    STRCMP_EQUAL("2009", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32PadsMicrosecondsToWidth6)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(formatter, 123, 6);

    LONGS_EQUAL(6, written);
    STRCMP_EQUAL("000123", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32ZeroPadsToFullWidth)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(formatter, 0, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("00", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, RemainingReturnsSpaceLeft)
{
    LONGS_EQUAL(TEST_BUFFER_SIZE, SolidSyslogFormatter_Remaining(formatter));

    SolidSyslogFormatter_Character(formatter, 'A');

    LONGS_EQUAL(TEST_BUFFER_SIZE - 1, SolidSyslogFormatter_Remaining(formatter));
}

TEST(SolidSyslogFormatter, LengthReturnsNumberOfCharactersWritten)
{
    LONGS_EQUAL(0, SolidSyslogFormatter_Length(formatter));

    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    LONGS_EQUAL(5, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, DataReturnsFormattedString)
{
    SolidSyslogFormatter_BoundedString(formatter, "test", 4);

    STRCMP_EQUAL("test", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, ZeroSizeBufferDoesNotCrash)
{
    SolidSyslogFormatterStorage  zeroStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(0)];
    struct SolidSyslogFormatter* zeroFormatter = SolidSyslogFormatter_Create(zeroStorage, 0);

    SolidSyslogFormatter_Character(zeroFormatter, 'A');

    LONGS_EQUAL(0, SolidSyslogFormatter_Length(zeroFormatter));
}

TEST(SolidSyslogFormatter, WritesUpToExactCapacity)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(4)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 4);

    SolidSyslogFormatter_BoundedString(small, "abcdef", 6);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("abc", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, CharacterStopsWhenFull)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(3)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 3);

    SolidSyslogFormatter_Character(small, 'A');
    SolidSyslogFormatter_Character(small, 'B');
    SolidSyslogFormatter_Character(small, 'C');

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("AB", SolidSyslogFormatter_Data(small));
}
