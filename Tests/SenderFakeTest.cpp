#include "CppUTest/TestHarness.h"
#include "SenderFake.h"
#include "SolidSyslogSender.h"
#include <cstring>

// clang-format off
TEST_GROUP(SenderFake)
{
    void setup() override
    {
        SenderFake_Reset();
    }
};

// clang-format on

TEST(SenderFake, CallCountIsZeroAfterReset)
{
    LONGS_EQUAL(0, SenderFake_CallCount());
}

TEST(SenderFake, CallCountIncrementsOnSend)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "a", 1);
    LONGS_EQUAL(1, SenderFake_CallCount());
}

TEST(SenderFake, CallCountIncrementsTwiceOnTwoSends)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "a", 1);
    SolidSyslogSender_Send(sender, "b", 1);
    LONGS_EQUAL(2, SenderFake_CallCount());
}

TEST(SenderFake, LastBufferCapturesMessage)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "hello", 5);
    STRCMP_EQUAL("hello", SenderFake_LastBufferAsString());
}

TEST(SenderFake, LastBufferIsNullTerminated)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "test", 4);
    LONGS_EQUAL(0, SenderFake_LastBufferAsString()[4]);
}

TEST(SenderFake, LastBufferCapturesLastSend)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "first", 5);
    SolidSyslogSender_Send(sender, "second", 6);
    STRCMP_EQUAL("second", SenderFake_LastBufferAsString());
}

TEST(SenderFake, SendReturnsTrue)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    CHECK_TRUE(SolidSyslogSender_Send(sender, "a", 1));
}

TEST(SenderFake, ResetClearsLastBuffer)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SolidSyslogSender_Send(sender, "hello", 5);
    SenderFake_Reset();
    STRCMP_EQUAL("", SenderFake_LastBufferAsString());
}

TEST(SenderFake, FailNextSendReturnsFalse)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SenderFake_FailNextSend();
    CHECK_FALSE(SolidSyslogSender_Send(sender, "a", 1));
}

TEST(SenderFake, FailNextSendStillCapturesBuffer)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SenderFake_FailNextSend();
    SolidSyslogSender_Send(sender, "hello", 5);
    STRCMP_EQUAL("hello", SenderFake_LastBufferAsString());
}

TEST(SenderFake, FailNextSendOnlyAffectsOneSend)
{
    struct SolidSyslogSender* sender = SenderFake_GetSender();
    SenderFake_FailNextSend();
    SolidSyslogSender_Send(sender, "a", 1);
    CHECK_TRUE(SolidSyslogSender_Send(sender, "b", 1));
}
