#include "CppUTest/TestHarness.h"
#include "SenderSpy.h"
#include "SolidSyslogSender.h"
#include <cstring>

TEST_GROUP(SenderSpy)
{
    void setup() override
    {
        SenderSpy_Reset();
    }
};

TEST(SenderSpy, CallCountIsZeroAfterReset)
{
    LONGS_EQUAL(0, SenderSpy_CallCount());
}

TEST(SenderSpy, CallCountIncrementsOnSend)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "a", 1);
    LONGS_EQUAL(1, SenderSpy_CallCount());
}

TEST(SenderSpy, CallCountIncrementsTwiceOnTwoSends)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "a", 1);
    SolidSyslogSender_Send(sender, "b", 1);
    LONGS_EQUAL(2, SenderSpy_CallCount());
}

TEST(SenderSpy, LastBufferCapturesMessage)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "hello", 5);
    STRCMP_EQUAL("hello", SenderSpy_LastBufferAsString());
}

TEST(SenderSpy, LastBufferIsNullTerminated)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "test", 4);
    LONGS_EQUAL(0, SenderSpy_LastBufferAsString()[4]);
}

TEST(SenderSpy, LastBufferCapturesLastSend)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "first", 5);
    SolidSyslogSender_Send(sender, "second", 6);
    STRCMP_EQUAL("second", SenderSpy_LastBufferAsString());
}

TEST(SenderSpy, ResetClearsLastBuffer)
{
    struct SolidSyslogSender* sender = SenderSpy_GetSender();
    SolidSyslogSender_Send(sender, "hello", 5);
    SenderSpy_Reset();
    STRCMP_EQUAL("", SenderSpy_LastBufferAsString());
}
