#include "CppUTest/TestHarness.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogStructuredData.h"

#include <cstdlib>
#include <cstring>

// clang-format off
TEST_GROUP(SolidSyslogOriginSd)
{
    SolidSyslogStructuredData* sd;
    char buffer[256];

    void setup() override
    {
        sd = SolidSyslogOriginSd_Create(malloc, "TestSoftware", "9.8.7");
    }

    void teardown() override
    {
        SolidSyslogOriginSd_Destroy(sd, free);
    }

    size_t format()
    {
        return SolidSyslogStructuredData_Format(sd, buffer, sizeof(buffer));
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

TEST(SolidSyslogOriginSd, FormatReturnsLength)
{
    size_t len = format();
    LONGS_EQUAL(strlen(buffer), len);
}

TEST(SolidSyslogOriginSd, DifferentValuesProduceDifferentOutput)
{
    SolidSyslogOriginSd_Destroy(sd, free);
    sd = SolidSyslogOriginSd_Create(malloc, "OtherSoft", "1.2.3");

    format();
    STRCMP_EQUAL("[origin software=\"OtherSoft\" swVersion=\"1.2.3\"]", buffer);
}
