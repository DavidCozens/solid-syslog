#include "CppUTest/TestHarness.h"
#include "SolidSyslogSender.h"
#include "SolidSyslogTcpSender.h"
#include "SocketFake.h"
#include <netinet/in.h>
#include <netinet/tcp.h>

static const char* const TEST_HOST        = "127.0.0.1";
static const int         TEST_PORT        = 6514;
static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;

static int GetPort()
{
    return TEST_PORT;
}

static const char* GetHost()
{
    return TEST_HOST;
}

// clang-format off
TEST_GROUP(SolidSyslogTcpSender)
{
    struct SolidSyslogTcpSenderConfig config = {GetPort, GetHost};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogTcpSender_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogTcpSender_Destroy();
    }

    void Send() const
    {
        SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogTcpSender, CreateReturnsNonNull)
{
    CHECK_TRUE(sender != nullptr);
}

TEST(SolidSyslogTcpSender, CreateOpensStreamSocket)
{
    LONGS_EQUAL(1, SocketFake_SocketCallCount());
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
    LONGS_EQUAL(SOCK_STREAM, SocketFake_SocketType());
}

TEST(SolidSyslogTcpSender, CreateSetsTcpNoDelay)
{
    LONGS_EQUAL(1, SocketFake_SetSockOptCallCount());
    LONGS_EQUAL(IPPROTO_TCP, SocketFake_LastSetSockOptLevel());
    LONGS_EQUAL(TCP_NODELAY, SocketFake_LastSetSockOptOptname());
}

// clang-format off
TEST_GROUP(SolidSyslogTcpSenderDestroy)
{
    // cppcheck-suppress unreadVariable -- used in test bodies; cppcheck does not model CppUTest macros
    struct SolidSyslogTcpSenderConfig config = {GetPort, GetHost};

    void setup() override { SocketFake_Reset(); }
    void teardown() override {}

    void CreateAndDestroy() const
    {
        SolidSyslogTcpSender_Create(&config);
        SolidSyslogTcpSender_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogTcpSenderDestroy, CloseCalledOnDestroy)
{
    CreateAndDestroy();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogTcpSenderDestroy, CloseCalledWithSocketFd)
{
    CreateAndDestroy();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastClosedFd());
}

TEST(SolidSyslogTcpSender, SendConnectsOnFirstCall)
{
    Send();
    LONGS_EQUAL(1, SocketFake_ConnectCallCount());
}

TEST(SolidSyslogTcpSender, SendConnectsWithCorrectPort)
{
    Send();
    LONGS_EQUAL(TEST_PORT, SocketFake_LastConnectPort());
}

TEST(SolidSyslogTcpSender, SendConnectsWithCorrectAddress)
{
    Send();
    STRCMP_EQUAL(TEST_HOST, SocketFake_LastConnectAddrAsString());
}

TEST(SolidSyslogTcpSender, SendConnectsWithSocketFd)
{
    Send();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastConnectFd());
}

TEST(SolidSyslogTcpSender, SecondSendDoesNotReconnect)
{
    Send();
    Send();
    LONGS_EQUAL(1, SocketFake_ConnectCallCount());
}

TEST(SolidSyslogTcpSender, SendTransmitsOctetCountingPrefix)
{
    Send();
    STRCMP_EQUAL("5 ", SocketFake_SendBufAsString(0));
}

TEST(SolidSyslogTcpSender, SendTransmitsMessageBody)
{
    Send();
    STRCMP_EQUAL(TEST_MESSAGE, SocketFake_SendBufAsString(1));
}

TEST(SolidSyslogTcpSender, SendMakesTwoSendCalls)
{
    Send();
    LONGS_EQUAL(2, SocketFake_SendCallCount());
}

TEST(SolidSyslogTcpSender, SendUsesSocketFd)
{
    Send();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastSendFd());
}

TEST(SolidSyslogTcpSender, SendReturnsTrueOnSuccess)
{
    CHECK_TRUE(SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN));
}
