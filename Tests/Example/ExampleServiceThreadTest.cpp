#include "CppUTest/TestHarness.h"
#include "ExampleServiceThread.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslogUdpSender.h"
#include "SocketSpy.h"
#include "ClockFake.h"

#include <cstdlib>

// clang-format off
TEST_GROUP(ExampleServiceThread)
{
    struct SolidSyslogSender* sender = nullptr;
    struct SolidSyslogBuffer* buffer = nullptr;
    struct SolidSyslog*       logger = nullptr;
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

        SolidSyslogConfig config = {buffer, sender, malloc, free, nullptr, nullptr, nullptr, nullptr};
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        logger = SolidSyslog_Create(&config);
    }

    void teardown() override
    {
        SolidSyslog_Destroy(logger);
        SolidSyslogPosixMqBuffer_Destroy(buffer);
        SolidSyslogUdpSender_Destroy(sender);
    }

    void Log() const
    {
        SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO, nullptr, nullptr};
        SolidSyslog_Log(logger, &message);
    }
};

// clang-format on

TEST(ExampleServiceThread, DoesNotSendWhenBufferEmpty)
{
    ExampleServiceThread_Run(logger, &shutdown);
    LONGS_EQUAL(0, SocketSpy_SendtoCallCount());
}

TEST(ExampleServiceThread, DrainsOneMessageAfterShutdown)
{
    Log();
    ExampleServiceThread_Run(logger, &shutdown);
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

TEST(ExampleServiceThread, DrainsMultipleMessagesAfterShutdown)
{
    Log();
    Log();
    Log();
    ExampleServiceThread_Run(logger, &shutdown);
    LONGS_EQUAL(3, SocketSpy_SendtoCallCount());
}
