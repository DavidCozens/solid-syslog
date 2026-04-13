#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogStructuredData.h"

#include <cstring>

enum
{
    TEST_BUFFER_SIZE = 256
};

// clang-format off
TEST_GROUP(SolidSyslogOriginSd)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogStructuredData* sd;
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(TEST_BUFFER_SIZE)];
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogFormatter* formatter;

    void setup() override
    {
        formatter = SolidSyslogFormatter_Create(storage, TEST_BUFFER_SIZE);
        sd = SolidSyslogOriginSd_Create("TestSoftware", "9.8.7");
    }

    void teardown() override
    {
        SolidSyslogOriginSd_Destroy();
    }

    void format() const
    {
        SolidSyslogStructuredData_Format(sd, formatter);
    }

    void resetFormatter()
    {
        formatter = SolidSyslogFormatter_Create(storage, TEST_BUFFER_SIZE);
    }
};

// clang-format on

TEST(SolidSyslogOriginSd, CreateReturnsNonNull)
{
    CHECK(sd != nullptr);
}

TEST(SolidSyslogOriginSd, FormatContainsSoftwareName)
{
    format();
    CHECK(strstr(SolidSyslogFormatter_Data(formatter), "software=\"TestSoftware\"") != nullptr);
}

TEST(SolidSyslogOriginSd, FormatContainsSwVersion)
{
    format();
    CHECK(strstr(SolidSyslogFormatter_Data(formatter), "swVersion=\"9.8.7\"") != nullptr);
}

TEST(SolidSyslogOriginSd, FormatProducesCompleteOriginSd)
{
    format();
    STRCMP_EQUAL("[origin software=\"TestSoftware\" swVersion=\"9.8.7\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, FormatAdvancesFormatterLength)
{
    LONGS_EQUAL(0, SolidSyslogFormatter_Length(formatter));
    format();
    CHECK(SolidSyslogFormatter_Length(formatter) > 0);
    LONGS_EQUAL(strlen(SolidSyslogFormatter_Data(formatter)), SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogOriginSd, DifferentValuesProduceDifferentOutput)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("OtherSoft", "1.2.3");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"OtherSoft\" swVersion=\"1.2.3\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, SoftwareAtMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl", "1.0");

    resetFormatter();
    format();
    CHECK(strstr(SolidSyslogFormatter_Data(formatter), "software=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl\"") != nullptr);
}

TEST(SolidSyslogOriginSd, SoftwareTruncatedBeyondMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklX", "1.0");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl\" swVersion=\"1.0\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, SwVersionAtMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");

    resetFormatter();
    format();
    CHECK(strstr(SolidSyslogFormatter_Data(formatter), "swVersion=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345\"") != nullptr);
}

TEST(SolidSyslogOriginSd, SwVersionTruncatedBeyondMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345X");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"S\" swVersion=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, EmptySoftwareString)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("", "1.0");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"\" swVersion=\"1.0\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, EmptySwVersionString)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"S\" swVersion=\"\"]", SolidSyslogFormatter_Data(formatter));
}

TEST(SolidSyslogOriginSd, NullSoftwareReturnsNull)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create(nullptr, "1.0");

    CHECK(sd == nullptr);
}

TEST(SolidSyslogOriginSd, NullSwVersionReturnsNull)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", nullptr);

    CHECK(sd == nullptr);
}

TEST(SolidSyslogOriginSd, DestroyDoesNotCrash)
{
    // Covered by teardown — this test documents the intent
}
