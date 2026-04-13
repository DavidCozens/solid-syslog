#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

TEST_GROUP(SolidSyslogFormatter)
{
    struct SolidSyslogFormatter formatter;
    char                        buffer[64];

    void setup() override
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
    }
};

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    size_t written = SolidSyslogFormatter_Character(&formatter, 'A');

    LONGS_EQUAL(1, written);
    STRCMP_EQUAL("A", buffer);
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    SolidSyslogFormatter_Character(&formatter, 'A');
    SolidSyslogFormatter_Character(&formatter, 'B');

    STRCMP_EQUAL("AB", buffer);
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    size_t written = SolidSyslogFormatter_BoundedString(&formatter, "hello", 10);

    LONGS_EQUAL(5, written);
    STRCMP_EQUAL("hello", buffer);
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    size_t written = SolidSyslogFormatter_BoundedString(&formatter, "hello", 3);

    LONGS_EQUAL(3, written);
    STRCMP_EQUAL("hel", buffer);
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(&formatter, '<');
    SolidSyslogFormatter_BoundedString(&formatter, "hello", 10);

    STRCMP_EQUAL("<hello", buffer);
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    size_t written = SolidSyslogFormatter_Uint32(&formatter, 7);

    LONGS_EQUAL(1, written);
    STRCMP_EQUAL("7", buffer);
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    SolidSyslogFormatter_Uint32(&formatter, 0);

    STRCMP_EQUAL("0", buffer);
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    size_t written = SolidSyslogFormatter_Uint32(&formatter, 134);

    LONGS_EQUAL(3, written);
    STRCMP_EQUAL("134", buffer);
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    SolidSyslogFormatter_Character(&formatter, '<');
    SolidSyslogFormatter_Uint32(&formatter, 42);

    STRCMP_EQUAL("<42", buffer);
}

TEST(SolidSyslogFormatter, PaddedUint32PadsSingleDigitToWidth2)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(&formatter, 5, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("05", buffer);
}

TEST(SolidSyslogFormatter, PaddedUint32NoPaddingWhenValueFillsWidth)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(&formatter, 12, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("12", buffer);
}

TEST(SolidSyslogFormatter, PaddedUint32PadsYearToWidth4)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(&formatter, 2009, 4);

    LONGS_EQUAL(4, written);
    STRCMP_EQUAL("2009", buffer);
}

TEST(SolidSyslogFormatter, PaddedUint32PadsMicrosecondsToWidth6)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(&formatter, 123, 6);

    LONGS_EQUAL(6, written);
    STRCMP_EQUAL("000123", buffer);
}

TEST(SolidSyslogFormatter, PaddedUint32ZeroPadsToFullWidth)
{
    size_t written = SolidSyslogFormatter_PaddedUint32(&formatter, 0, 2);

    LONGS_EQUAL(2, written);
    STRCMP_EQUAL("00", buffer);
}

static size_t StubWriteHello(char* buf, size_t size)
{
    (void) size;
    const char* text = "hello";
    size_t      len  = 5;
    memcpy(buf, text, len + 1);
    return len;
}

TEST(SolidSyslogFormatter, CallbackInvokesFunction)
{
    SolidSyslogFormatter_Character(&formatter, '<');
    size_t written = SolidSyslogFormatter_Callback(&formatter, StubWriteHello, 10);

    LONGS_EQUAL(5, written);
    STRCMP_EQUAL("<hello", buffer);
}

TEST(SolidSyslogFormatter, RemainingReturnsSpaceLeft)
{
    LONGS_EQUAL(64, SolidSyslogFormatter_Remaining(&formatter));

    SolidSyslogFormatter_Character(&formatter, 'A');

    LONGS_EQUAL(63, SolidSyslogFormatter_Remaining(&formatter));
}
