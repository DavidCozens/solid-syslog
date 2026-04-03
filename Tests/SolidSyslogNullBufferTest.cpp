#include "CppUTest/TestHarness.h"
#include "SolidSyslogNullBuffer.h"
#include "SenderSpy.h"

// clang-format off
TEST_GROUP(SolidSyslogNullBuffer)
{
    struct SolidSyslogBuffer* buffer = nullptr;

    void setup() override
    {
        SenderSpy_Reset();
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        buffer = SolidSyslogNullBuffer_Create(SenderSpy_GetSender());
    }

    void teardown() override
    {
        SolidSyslogNullBuffer_Destroy(buffer);
    }
};

// clang-format on

TEST(SolidSyslogNullBuffer, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogNullBuffer, WriteForwardsToSender)
{
    SolidSyslogBuffer_Write(buffer, "hello", 5);
    LONGS_EQUAL(1, SenderSpy_CallCount());
    STRCMP_EQUAL("hello", SenderSpy_LastBufferAsString());
}
