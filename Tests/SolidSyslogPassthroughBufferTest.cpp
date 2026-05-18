#include <stddef.h>

#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting; // NOLINT(google-build-using-namespace) -- test-file scope only; brings NEVER/ONCE/TWICE/THRICE into scope for the CALLED_*
    // macros
#include "SolidSyslogBuffer.h"
#include "SolidSyslogPassthroughBuffer.h"
#include "SenderFake.h"

static const char* const TEST_MESSAGE = "hello";
static const size_t TEST_MESSAGE_LEN = 5;

// clang-format off
TEST_GROUP(SolidSyslogPassthroughBuffer)
{
    struct SolidSyslogSender* fakeSender = nullptr;
    struct SolidSyslogBuffer* buffer     = nullptr;

    void setup() override
    {
        fakeSender = SenderFake_Create();
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        buffer = SolidSyslogPassthroughBuffer_Create(fakeSender);
    }

    void teardown() override
    {
        SolidSyslogPassthroughBuffer_Destroy();
        SenderFake_Destroy(fakeSender);
    }

    void Write() const
    {
        SolidSyslogBuffer_Write(buffer, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogPassthroughBuffer, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogPassthroughBuffer, WriteForwardsBufferToSender)
{
    Write();
    STRCMP_EQUAL(TEST_MESSAGE, SenderFake_LastBufferAsString(fakeSender));
}

TEST(SolidSyslogPassthroughBuffer, WriteForwardsSizeToSender)
{
    Write();
    LONGS_EQUAL(TEST_MESSAGE_LEN, SenderFake_LastSize(fakeSender));
}

TEST(SolidSyslogPassthroughBuffer, WriteResultsInOneSend)
{
    Write();
    CALLED_FAKE_ON(SenderFake_Send, fakeSender, ONCE);
}

TEST(SolidSyslogPassthroughBuffer, TwoWritesResultInTwoSends)
{
    Write();
    Write();
    CALLED_FAKE_ON(SenderFake_Send, fakeSender, TWICE);
}

TEST(SolidSyslogPassthroughBuffer, NoWritesResultInNoSends)
{
    CALLED_FAKE_ON(SenderFake_Send, fakeSender, NEVER);
}

TEST(SolidSyslogPassthroughBuffer, ReadReturnsNothingToSend)
{
    char data[512];
    size_t bytesRead = 0;
    bool sent = SolidSyslogBuffer_Read(buffer, data, sizeof(data), &bytesRead);
    CHECK_FALSE(sent);
}

IGNORE_TEST(SolidSyslogPassthroughBuffer, HappyPathOnly)

{
    // Error handling not yet implemented — see Epic #31
    //   Create with NULL sender returns NULL
    //   Write with NULL buffer does not crash
    //   Destroy with NULL buffer does not crash
}
