#include "CppUTest/TestHarness.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogStructuredData.h"

// clang-format off
TEST_GROUP(SolidSyslogMetaSd)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogAtomicCounter* counter;
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogStructuredData* sd;
    char buffer[256];
    SolidSyslogFormatter formatter;

    void setup() override
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
        counter = SolidSyslogAtomicCounter_Create();
        sd = SolidSyslogMetaSd_Create(counter);
    }

    void teardown() override
    {
        SolidSyslogMetaSd_Destroy();
        SolidSyslogAtomicCounter_Destroy();
    }

    void format()
    {
        SolidSyslogStructuredData_Format(sd, &formatter);
    }

    void resetFormatter()
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
    }
};

// clang-format on

TEST(SolidSyslogMetaSd, CreateReturnsNonNull)
{
    CHECK(sd != nullptr);
}

TEST(SolidSyslogMetaSd, FirstFormatProducesSequenceId1)
{
    format();
    STRCMP_EQUAL("[meta sequenceId=\"1\"]", buffer);
}

TEST(SolidSyslogMetaSd, SecondFormatProducesSequenceId2)
{
    format();
    resetFormatter();
    format();
    STRCMP_EQUAL("[meta sequenceId=\"2\"]", buffer);
}

TEST(SolidSyslogMetaSd, ThirdFormatProducesSequenceId3)
{
    format();
    format();
    resetFormatter();
    format();
    STRCMP_EQUAL("[meta sequenceId=\"3\"]", buffer);
}

TEST(SolidSyslogMetaSd, FormatAdvancesFormatterPosition)
{
    format();
    LONGS_EQUAL(strlen(buffer), formatter.position);
}

TEST(SolidSyslogMetaSd, DestroyDoesNotCrash)
{
    // Covered by teardown — this test documents the intent
}
