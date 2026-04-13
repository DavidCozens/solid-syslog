#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "StringFake.h"

// clang-format off
TEST_GROUP(StringFake)
{
    char                 buffer[32];
    SolidSyslogFormatter formatter;

    void setup() override
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
        StringFake_Reset();
    }
};

// clang-format on

TEST(StringFake, ReturnsEmptyStringAfterReset)
{
    StringFake_GetHostname(&formatter);
    STRCMP_EQUAL("", buffer);
    LONGS_EQUAL(0, formatter.position);
}

TEST(StringFake, ReturnsConfiguredHostname)
{
    StringFake_SetHostname("MyHost");
    StringFake_GetHostname(&formatter);
    STRCMP_EQUAL("MyHost", buffer);
    LONGS_EQUAL(6, formatter.position);
}
