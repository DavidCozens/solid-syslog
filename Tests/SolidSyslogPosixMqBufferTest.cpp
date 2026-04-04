#include "CppUTest/TestHarness.h"
#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SenderSpy.h"
#include <cstdlib>

static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;

// clang-format off
TEST_GROUP(SolidSyslogPosixMqBuffer)
{
    struct SolidSyslogBuffer* buffer = nullptr;
    char   readData[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    size_t readSize;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        buffer = SolidSyslogPosixMqBuffer_Create(SOLIDSYSLOG_MAX_MESSAGE_SIZE, 10);
        readSize = 0;
    }

    void teardown() override
    {
        SolidSyslogPosixMqBuffer_Destroy(buffer);
    }

    void Write() const
    {
        SolidSyslogBuffer_Write(buffer, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }

    bool Read()
    {
        return SolidSyslogBuffer_Read(buffer, readData, sizeof(readData), &readSize);
    }
};

// clang-format on

TEST(SolidSyslogPosixMqBuffer, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogPosixMqBuffer, WriteAndReadReturnsTrue)
{
    Write();
    CHECK_TRUE(Read());
}

TEST(SolidSyslogPosixMqBuffer, ReadReturnsWrittenData)
{
    Write();
    Read();
    MEMCMP_EQUAL(TEST_MESSAGE, readData, TEST_MESSAGE_LEN);
}

TEST(SolidSyslogPosixMqBuffer, ReadReturnsWrittenSize)
{
    Write();
    Read();
    LONGS_EQUAL(TEST_MESSAGE_LEN, readSize);
}

TEST(SolidSyslogPosixMqBuffer, ReadFromEmptyReturnsFalse)
{
    CHECK_FALSE(Read());
}

TEST(SolidSyslogPosixMqBuffer, MultipleWritesReadInOrder)
{
    SolidSyslogBuffer_Write(buffer, "first", 5);
    SolidSyslogBuffer_Write(buffer, "second", 6);
    Read();
    MEMCMP_EQUAL("first", readData, 5);
    Read();
    MEMCMP_EQUAL("second", readData, 6);
}

TEST(SolidSyslogPosixMqBuffer, SecondReadAfterSingleWriteReturnsFalse)
{
    Write();
    Read();
    CHECK_FALSE(Read());
}

TEST(SolidSyslogPosixMqBuffer, ServiceSendsMessageWrittenViaLog)
{
    SenderSpy_Reset();
    SolidSyslogConfig config = {buffer, SenderSpy_GetSender(), malloc, free, nullptr, nullptr, nullptr, nullptr};
    SolidSyslog*      logger = SolidSyslog_Create(&config);

    SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO, nullptr, nullptr};
    SolidSyslog_Log(logger, &message);
    CHECK_TRUE(SolidSyslog_Service(logger));
    LONGS_EQUAL(1, SenderSpy_CallCount());

    SolidSyslog_Destroy(logger);
}

IGNORE_TEST(SolidSyslogPosixMqBuffer, HappyPathOnly)
{
    // Error handling not yet implemented — see Epic #31
    //   Create with zero maxMessageSize or maxMessages
    //   Create when mq_open fails returns NULL
    //   Write when queue is full (back-pressure / overflow)
    //   Destroy with NULL buffer does not crash
    //
    // Blocking mode not yet implemented — see S4.5 or later
    //   Read blocks waiting for a message (O_NONBLOCK removed)
}
