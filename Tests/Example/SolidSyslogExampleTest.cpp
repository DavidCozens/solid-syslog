#include "CppUTest/TestHarness.h"
#include "SolidSyslogExample.h"

// clang-format off
TEST_GROUP(SolidSyslogExample)
{
};

// clang-format on

TEST(SolidSyslogExample, InvalidOptionReturnsOne)
{
    char  arg0[]   = "SolidSyslogExample";
    char  arg1[]   = "--invalid";
    char* argv[]   = {arg0, arg1, nullptr};
    int   argc     = 2;
    LONGS_EQUAL(1, SolidSyslogExample_Run(argc, argv));
}
