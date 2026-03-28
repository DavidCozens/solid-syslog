#include "CppUTest/TestHarness.h"
#include "SolidSyslog.h"

// clang-format off
TEST_GROUP(SolidSyslog)
{
    void setup() override
    {
        SolidSyslog_Create();
    }

    void teardown() override
    {
        SolidSyslog_Destroy();
    }
};
// clang-format on

TEST(SolidSyslog, NeedsWork)
{
    CHECK(true);
}
