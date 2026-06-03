#include "CppUTest/TestHarness.h"
#include "TestUtils.h"

extern "C"
{
#include "PlusFatFake.h"
#include "SolidSyslogFile.h"
#include "SolidSyslogPlusFatFile.h"
#include "ff_stdio.h"
}

using namespace CososoTesting;

static const char* const TEST_PATH = "test.log";

// clang-format off
TEST_GROUP(SolidSyslogPlusFatFile)
{
    struct SolidSyslogFile* file = nullptr;

    void setup() override
    {
        PlusFatFake_Reset();
        file = SolidSyslogPlusFatFile_Create();
    }

    void teardown() override
    {
        SolidSyslogPlusFatFile_Destroy(file);
    }
};

// clang-format on

TEST(SolidSyslogPlusFatFile, CreateSucceeds)
{
    CHECK(file != nullptr);
}
