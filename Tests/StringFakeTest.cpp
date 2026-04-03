#include "CppUTest/TestHarness.h"
#include "StringFake.h"

// clang-format off
TEST_GROUP(StringFake)
{
    void setup() override
    {
        StringFake_Reset();
    }
};

// clang-format on

TEST(StringFake, ReturnsEmptyStringAfterReset)
{
    char   buffer[32];
    size_t len = StringFake_GetHostname(buffer, sizeof(buffer));
    STRCMP_EQUAL("", buffer);
    LONGS_EQUAL(0, len);
}

TEST(StringFake, ReturnsConfiguredHostname)
{
    StringFake_SetHostname("MyHost");
    char   buffer[32];
    size_t len = StringFake_GetHostname(buffer, sizeof(buffer));
    STRCMP_EQUAL("MyHost", buffer);
    LONGS_EQUAL(6, len);
}

TEST(StringFake, TruncatesWhenBufferTooSmall)
{
    StringFake_SetHostname("LongHostname");
    char   buffer[5];
    size_t len = StringFake_GetHostname(buffer, sizeof(buffer));
    STRCMP_EQUAL("Long", buffer);
    LONGS_EQUAL(4, len);
}
