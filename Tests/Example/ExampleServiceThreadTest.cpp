#include "CppUTest/TestHarness.h"
#include "ExampleServiceThread.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslogUdpSender.h"
#include "SocketSpy.h"
#include "ClockFake.h"

// clang-format off
TEST_GROUP(ExampleServiceThread)
{
    struct SolidSyslogSender* sender = nullptr;
    struct SolidSyslogBuffer* buffer = nullptr;
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    volatile bool             shutdown;

    void setup() override
    {
        SocketSpy_Reset();
        ClockFake_Reset();
        ClockFake_SetTime(1743768600, 0);
        shutdown = true;

        SolidSyslogUdpSenderConfig udpConfig = {ExampleUdpConfig_GetPort, ExampleUdpConfig_GetHost};
        sender = SolidSyslogUdpSender_Create(&udpConfig);
        buffer = SolidSyslogPosixMqBuffer_Create(SOLIDSYSLOG_MAX_MESSAGE_SIZE, 10);

        SolidSyslogConfig config = {buffer, sender, nullptr, nullptr, nullptr, nullptr, nullptr, 0};
        SolidSyslog_Create(&config);
    }

    void teardown() override
    {
        SolidSyslog_Destroy();
        SolidSyslogPosixMqBuffer_Destroy();
        SolidSyslogUdpSender_Destroy();
    }

    static void Log()
    {
        SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO, nullptr, nullptr};
        SolidSyslog_Log(&message);
    }
};

// clang-format on

TEST(ExampleServiceThread, DoesNotSendWhenBufferEmpty)
{
    ExampleServiceThread_Run(&shutdown);
    LONGS_EQUAL(0, SocketSpy_SendtoCallCount());
}

TEST(ExampleServiceThread, DrainsOneMessageAfterShutdown)
{
    Log();
    ExampleServiceThread_Run(&shutdown);
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

TEST(ExampleServiceThread, DrainsMultipleMessagesAfterShutdown)
{
    Log();
    Log();
    Log();
    ExampleServiceThread_Run(&shutdown);
    LONGS_EQUAL(3, SocketSpy_SendtoCallCount());
}
