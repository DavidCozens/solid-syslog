#include "CppUTest/TestHarness.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogStructuredData.h"

#include <cstdlib>

// clang-format off
TEST_GROUP(SolidSyslogMetaSd)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogAtomicCounter* counter;
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogStructuredData* sd;
    char buffer[256];

    void setup() override
    {
        counter = SolidSyslogAtomicCounter_Create(malloc);
        sd = SolidSyslogMetaSd_Create(malloc, counter);
    }

    void teardown() override
    {
        SolidSyslogMetaSd_Destroy(sd, free);
        SolidSyslogAtomicCounter_Destroy(counter, free);
    }

    size_t format()
    {
        return SolidSyslogStructuredData_Format(sd, buffer, sizeof(buffer));
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
    format();
    STRCMP_EQUAL("[meta sequenceId=\"2\"]", buffer);
}

TEST(SolidSyslogMetaSd, ThirdFormatProducesSequenceId3)
{
    format();
    format();
    format();
    STRCMP_EQUAL("[meta sequenceId=\"3\"]", buffer);
}

TEST(SolidSyslogMetaSd, FormatReturnsLengthOfFormattedString)
{
    size_t len = format();
    LONGS_EQUAL(strlen(buffer), len);
}

TEST(SolidSyslogMetaSd, DestroyDoesNotCrash)
{
    // Covered by teardown — this test documents the intent
}
