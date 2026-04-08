#include "CppUTest/TestHarness.h"
#include "SolidSyslogNullBuffer.h"
#include "SenderFake.h"

static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;

// clang-format off
TEST_GROUP(SolidSyslogNullBuffer)
{
    struct SolidSyslogBuffer* buffer = nullptr;

    void setup() override
    {
        SenderFake_Reset();
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        buffer = SolidSyslogNullBuffer_Create(SenderFake_GetSender());
    }

    void teardown() override
    {
        SolidSyslogNullBuffer_Destroy();
    }

    void Write() const
    {
        SolidSyslogBuffer_Write(buffer, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogNullBuffer, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogNullBuffer, WriteForwardsBufferToSender)
{
    Write();
    STRCMP_EQUAL(TEST_MESSAGE, SenderFake_LastBufferAsString());
}

TEST(SolidSyslogNullBuffer, WriteForwardsSizeToSender)
{
    Write();
    LONGS_EQUAL(TEST_MESSAGE_LEN, SenderFake_LastSize());
}

TEST(SolidSyslogNullBuffer, WriteResultsInOneSend)
{
    Write();
    LONGS_EQUAL(1, SenderFake_CallCount());
}

TEST(SolidSyslogNullBuffer, TwoWritesResultInTwoSends)
{
    Write();
    Write();
    LONGS_EQUAL(2, SenderFake_CallCount());
}

TEST(SolidSyslogNullBuffer, NoWritesResultInNoSends)
{
    LONGS_EQUAL(0, SenderFake_CallCount());
}

TEST(SolidSyslogNullBuffer, ReadReturnsNothingToSend)
{
    char   data[512];
    size_t bytesRead = 0;
    bool   sent      = SolidSyslogBuffer_Read(buffer, data, sizeof(data), &bytesRead);
    CHECK_FALSE(sent);
}

IGNORE_TEST(SolidSyslogNullBuffer, HappyPathOnly)
{
    // Error handling not yet implemented — see Epic #31
    //   Create with NULL sender returns NULL
    //   Write with NULL buffer does not crash
    //   Destroy with NULL buffer does not crash
}
