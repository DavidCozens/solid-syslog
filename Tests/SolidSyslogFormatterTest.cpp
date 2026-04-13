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
    SolidSyslogFormatter_Character(formatter, 'A');

    LONGS_EQUAL(1, SolidSyslogFormatter_Length(formatter));
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
    SolidSyslogFormatter_BoundedString(formatter, "hello", 10);

    LONGS_EQUAL(5, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("hello", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    SolidSyslogFormatter_BoundedString(formatter, "hello", 3);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(formatter));
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
    SolidSyslogFormatter_Uint32(formatter, 7);

    LONGS_EQUAL(1, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("7", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    SolidSyslogFormatter_Uint32(formatter, 0);

    STRCMP_EQUAL("0", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    SolidSyslogFormatter_Uint32(formatter, 134);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(formatter));
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
    SolidSyslogFormatter_PaddedUint32(formatter, 5, 2);

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("05", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32NoPaddingWhenValueFillsWidth)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 12, 2);

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("12", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32PadsYearToWidth4)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 2009, 4);

    LONGS_EQUAL(4, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("2009", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32PadsMicrosecondsToWidth6)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 123, 6);

    LONGS_EQUAL(6, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("000123", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogFormatter, PaddedUint32ZeroPadsToFullWidth)
{
    SolidSyslogFormatter_PaddedUint32(formatter, 0, 2);

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
    STRCMP_EQUAL("00", SolidSyslogFormatter_Data(formatter));
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

TEST(SolidSyslogFormatter, CharacterFillsExactlyOneByteBuffer)
{
    SolidSyslogFormatterStorage  tinyStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(1)];
    struct SolidSyslogFormatter* tiny = SolidSyslogFormatter_Create(tinyStorage, 1);

    SolidSyslogFormatter_Character(tiny, 'X');

    LONGS_EQUAL(0, SolidSyslogFormatter_Length(tiny));
    STRCMP_EQUAL("", SolidSyslogFormatter_Data(tiny));
}

TEST(SolidSyslogFormatter, BoundedStringStopsAtBufferCapacity)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(4)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 4);

    SolidSyslogFormatter_BoundedString(small, "abcdef", 10);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("abc", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, BoundedStringWritesNothingWhenFull)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(4)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 4);

    SolidSyslogFormatter_BoundedString(small, "abc", 3);
    SolidSyslogFormatter_BoundedString(small, "xyz", 3);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("abc", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, Uint32TruncatedWhenBufferTooSmall)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(3)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 3);

    SolidSyslogFormatter_Uint32(small, 12345);

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("12", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, Uint32FitsExactly)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(4)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 4);

    SolidSyslogFormatter_Uint32(small, 123);

    LONGS_EQUAL(3, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("123", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, PaddedUint32TruncatedWhenBufferTooSmall)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(3)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 3);

    SolidSyslogFormatter_PaddedUint32(small, 5, 4);

    LONGS_EQUAL(2, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("00", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, PaddedUint32FitsExactly)
{
    SolidSyslogFormatterStorage  smallStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(5)];
    struct SolidSyslogFormatter* small = SolidSyslogFormatter_Create(smallStorage, 5);

    SolidSyslogFormatter_PaddedUint32(small, 9, 4);

    LONGS_EQUAL(4, SolidSyslogFormatter_Length(small));
    STRCMP_EQUAL("0009", SolidSyslogFormatter_Data(small));
}

TEST(SolidSyslogFormatter, ZeroSizeBoundedStringIsNoOp)
{
    SolidSyslogFormatterStorage  zeroStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(0)];
    struct SolidSyslogFormatter* zero = SolidSyslogFormatter_Create(zeroStorage, 0);

    SolidSyslogFormatter_BoundedString(zero, "hello", 5);

    LONGS_EQUAL(0, SolidSyslogFormatter_Length(zero));
}

TEST(SolidSyslogFormatter, ZeroSizeUint32IsNoOp)
{
    SolidSyslogFormatterStorage  zeroStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(0)];
    struct SolidSyslogFormatter* zero = SolidSyslogFormatter_Create(zeroStorage, 0);

    SolidSyslogFormatter_Uint32(zero, 42);

    LONGS_EQUAL(0, SolidSyslogFormatter_Length(zero));
}
