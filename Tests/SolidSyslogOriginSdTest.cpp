#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogStructuredData.h"

#include <cstring>

// clang-format off
TEST_GROUP(SolidSyslogOriginSd)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogStructuredData* sd;
    char buffer[256];
    SolidSyslogFormatter formatter;

    void setup() override
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
        sd = SolidSyslogOriginSd_Create("TestSoftware", "9.8.7");
    }

    void teardown() override
    {
        SolidSyslogOriginSd_Destroy();
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

TEST(SolidSyslogOriginSd, CreateReturnsNonNull)
{
    CHECK(sd != nullptr);
}

TEST(SolidSyslogOriginSd, FormatContainsSoftwareName)
{
    format();
    CHECK(strstr(buffer, "software=\"TestSoftware\"") != nullptr);
}

TEST(SolidSyslogOriginSd, FormatContainsSwVersion)
{
    format();
    CHECK(strstr(buffer, "swVersion=\"9.8.7\"") != nullptr);
}

TEST(SolidSyslogOriginSd, FormatProducesCompleteOriginSd)
{
    format();
    STRCMP_EQUAL("[origin software=\"TestSoftware\" swVersion=\"9.8.7\"]", buffer);
}

TEST(SolidSyslogOriginSd, FormatAdvancesFormatterPosition)
{
    format();
    LONGS_EQUAL(strlen(buffer), formatter.position);
}

TEST(SolidSyslogOriginSd, DifferentValuesProduceDifferentOutput)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("OtherSoft", "1.2.3");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"OtherSoft\" swVersion=\"1.2.3\"]", buffer);
}

TEST(SolidSyslogOriginSd, SoftwareAtMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl", "1.0");

    resetFormatter();
    format();
    CHECK(strstr(buffer, "software=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl\"") != nullptr);
}

TEST(SolidSyslogOriginSd, SoftwareTruncatedBeyondMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklX", "1.0");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl\" swVersion=\"1.0\"]", buffer);
}

TEST(SolidSyslogOriginSd, SwVersionAtMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");

    resetFormatter();
    format();
    CHECK(strstr(buffer, "swVersion=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345\"") != nullptr);
}

TEST(SolidSyslogOriginSd, SwVersionTruncatedBeyondMaxLength)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345X");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"S\" swVersion=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345\"]", buffer);
}

TEST(SolidSyslogOriginSd, EmptySoftwareString)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("", "1.0");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"\" swVersion=\"1.0\"]", buffer);
}

TEST(SolidSyslogOriginSd, EmptySwVersionString)
{
    SolidSyslogOriginSd_Destroy();
    sd = SolidSyslogOriginSd_Create("S", "");

    resetFormatter();
    format();
    STRCMP_EQUAL("[origin software=\"S\" swVersion=\"\"]", buffer);
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
