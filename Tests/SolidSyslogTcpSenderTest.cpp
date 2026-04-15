#include "CppUTest/TestHarness.h"
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogSender.h"
#include "SolidSyslogTcpSender.h"
#include "SocketFake.h"
#include <netinet/in.h>
#include <netinet/tcp.h>

// clang-format off
static const char* const TEST_HOST           = "127.0.0.1";
static const int         TEST_PORT           = 514;
static const char* const TEST_ALTERNATE_HOST = "192.168.1.1";
static const int         TEST_ALTERNATE_PORT = 9999;
static const char* const TEST_MESSAGE        = "hello";
static const size_t      TEST_MESSAGE_LEN    = 5;
// clang-format on

static int GetPort()
{
    return TEST_PORT;
}

static const char* GetHost()
{
    return TEST_HOST;
}

static int GetAlternatePort()
{
    return TEST_ALTERNATE_PORT;
}

static const char* GetAlternateHost()
{
    return TEST_ALTERNATE_HOST;
}

static int getPortCallCount;

static int SpyGetPort()
{
    getPortCallCount++;
    return TEST_PORT;
}

static int getHostCallCount;

static const char* SpyGetHost()
{
    getHostCallCount++;
    return TEST_HOST;
}

// clang-format off
TEST_GROUP(SolidSyslogTcpSender)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct SolidSyslogTcpSenderConfig config;
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetPort);
        config = {resolver};
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogTcpSender_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogTcpSender_Destroy();
        SolidSyslogGetAddrInfoResolver_Destroy();
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

TEST(SolidSyslogTcpSender, CreateDoesNotOpenSocket)
{
    LONGS_EQUAL(0, SocketFake_SocketCallCount());
}

TEST(SolidSyslogTcpSender, FirstSendOpensStreamSocket)
{
    Send();
    LONGS_EQUAL(1, SocketFake_SocketCallCount());
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
    LONGS_EQUAL(SOCK_STREAM, SocketFake_SocketType());
}

TEST(SolidSyslogTcpSender, FirstSendSetsTcpNoDelay)
{
    Send();
    LONGS_EQUAL(1, SocketFake_SetSockOptCallCount());
    LONGS_EQUAL(IPPROTO_TCP, SocketFake_LastSetSockOptLevel());
    LONGS_EQUAL(TCP_NODELAY, SocketFake_LastSetSockOptOptname());
}

// clang-format off
TEST_GROUP(SolidSyslogTcpSenderDestroy)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct SolidSyslogTcpSenderConfig config;

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetPort);
        // cppcheck-suppress unreadVariable -- used in test bodies; cppcheck does not model CppUTest macros
        config = {resolver};
    }

    void teardown() override
    {
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void CreateAndDestroy() const
    {
        SolidSyslogTcpSender_Create(&config);
        SolidSyslogTcpSender_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogTcpSenderDestroy, DestroyWithoutSendDoesNotClose)
{
    CreateAndDestroy();
    LONGS_EQUAL(0, SocketFake_CloseCallCount());
}

TEST(SolidSyslogTcpSenderDestroy, DestroyAfterSendClosesSocket)
{
    struct SolidSyslogSender* sender = SolidSyslogTcpSender_Create(&config);
    SolidSyslogSender_Send(sender, "x", 1);
    SolidSyslogTcpSender_Destroy();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
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

// clang-format off
TEST_GROUP(SolidSyslogTcpSenderConfig)
{
    // cppcheck-suppress unreadVariable -- assigned in CreateSender; cppcheck does not model CppUTest macros
    const char* (*getHostFn)(void) = GetHost; // NOLINT(modernize-redundant-void-arg) -- C idiom
    // cppcheck-suppress unreadVariable -- assigned in CreateSender; cppcheck does not model CppUTest macros
    int (*getPortFn)(void) = GetPort; // NOLINT(modernize-redundant-void-arg) -- C idiom
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        getPortCallCount = 0;
        getHostCallCount = 0;
    }

    void teardown() override
    {
        SolidSyslogTcpSender_Destroy();
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void CreateSender()
    {
        struct SolidSyslogResolver* resolver = SolidSyslogGetAddrInfoResolver_Create(getHostFn, getPortFn);
        struct SolidSyslogTcpSenderConfig config = {resolver};
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogTcpSender_Create(&config);
    }

    void Send() const
    {
        SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogTcpSenderConfig, GetPortCalledOnFirstSend)
{
    getPortFn = SpyGetPort;
    CreateSender();
    LONGS_EQUAL(0, getPortCallCount);
    Send();
    LONGS_EQUAL(1, getPortCallCount);
}

TEST(SolidSyslogTcpSenderConfig, GetPortNotCalledOnSecondSend)
{
    getPortFn = SpyGetPort;
    CreateSender();
    Send();
    Send();
    LONGS_EQUAL(1, getPortCallCount);
}

TEST(SolidSyslogTcpSenderConfig, ConnectsWithAlternatePort)
{
    getPortFn = GetAlternatePort;
    CreateSender();
    Send();
    LONGS_EQUAL(TEST_ALTERNATE_PORT, SocketFake_LastConnectPort());
}

TEST(SolidSyslogTcpSenderConfig, GetHostCalledOnFirstSend)
{
    getHostFn = SpyGetHost;
    CreateSender();
    LONGS_EQUAL(0, getHostCallCount);
    Send();
    LONGS_EQUAL(1, getHostCallCount);
}

TEST(SolidSyslogTcpSenderConfig, GetHostNotCalledOnSecondSend)
{
    getHostFn = SpyGetHost;
    CreateSender();
    Send();
    Send();
    LONGS_EQUAL(1, getHostCallCount);
}

TEST(SolidSyslogTcpSenderConfig, ConnectsWithAlternateHost)
{
    getHostFn = GetAlternateHost;
    CreateSender();
    Send();
    STRCMP_EQUAL(TEST_ALTERNATE_HOST, SocketFake_LastConnectAddrAsString());
}

TEST(SolidSyslogTcpSenderConfig, GetAddrInfoCalledWithHostname)
{
    getHostFn = SpyGetHost;
    CreateSender();
    Send();
    STRCMP_EQUAL(TEST_HOST, SocketFake_LastGetAddrInfoHostname());
}

// clang-format off
TEST_GROUP(SolidSyslogTcpSenderFailure)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct SolidSyslogTcpSenderConfig config;
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetPort);
        config = {resolver};
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogTcpSender_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogTcpSender_Destroy();
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    // NOLINTNEXTLINE(modernize-use-nodiscard) -- test helper; return value intentionally ignored in some tests
    bool Send() const
    {
        return SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogTcpSenderFailure, SendReturnsFalseWhenConnectFails)
{
    SocketFake_SetConnectFails(true);
    CHECK_FALSE(Send());
}

TEST(SolidSyslogTcpSenderFailure, ConnectFailureClosesSocket)
{
    SocketFake_SetConnectFails(true);
    Send();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogTcpSenderFailure, SendReturnsFalseWhenSendFails)
{
    SocketFake_SetSendFails(true);
    CHECK_FALSE(Send());
}

TEST(SolidSyslogTcpSenderFailure, SendFailureClosesSocket)
{
    SocketFake_SetSendFails(true);
    Send();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogTcpSenderFailure, DestroyAfterSendFailureDoesNotDoubleClose)
{
    SocketFake_SetSendFails(true);
    Send();
    SolidSyslogTcpSender_Destroy();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogTcpSenderFailure, SendFailureMarksDisconnected)
{
    Send();
    LONGS_EQUAL(1, SocketFake_ConnectCallCount());
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    Send();
    LONGS_EQUAL(2, SocketFake_ConnectCallCount());
}

TEST(SolidSyslogTcpSenderFailure, ReconnectCreatesNewSocket)
{
    Send();
    int firstSocketCallCount = SocketFake_SocketCallCount();
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    Send();
    LONGS_EQUAL(firstSocketCallCount + 1, SocketFake_SocketCallCount());
}

TEST(SolidSyslogTcpSenderFailure, ReconnectSetsTcpNoDelay)
{
    Send();
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    Send();
    LONGS_EQUAL(2, SocketFake_SetSockOptCallCount());
    LONGS_EQUAL(IPPROTO_TCP, SocketFake_LastSetSockOptLevel());
    LONGS_EQUAL(TCP_NODELAY, SocketFake_LastSetSockOptOptname());
}

TEST(SolidSyslogTcpSenderFailure, ReconnectResolvesDns)
{
    Send();
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    Send();
    LONGS_EQUAL(2, SocketFake_GetAddrInfoCallCount());
}

TEST(SolidSyslogTcpSenderFailure, ReconnectConnectsWithNewFd)
{
    Send();
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    Send();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastConnectFd());
}

TEST(SolidSyslogTcpSenderFailure, SuccessfulSendAfterReconnect)
{
    Send();
    SocketFake_SetSendFails(true);
    Send();
    SocketFake_SetSendFails(false);
    CHECK_TRUE(Send());
}

TEST(SolidSyslogTcpSenderFailure, SendUsesMsgNoSignal)
{
    Send();
    LONGS_EQUAL(MSG_NOSIGNAL, SocketFake_SendFlags(0));
    LONGS_EQUAL(MSG_NOSIGNAL, SocketFake_SendFlags(1));
}

TEST(SolidSyslogTcpSenderFailure, SendReturnsFalseWhenBodySendFails)
{
    SocketFake_FailSendOnCall(1);
    CHECK_FALSE(Send());
}

TEST(SolidSyslogTcpSenderFailure, BodySendFailureClosesSocket)
{
    SocketFake_FailSendOnCall(1);
    Send();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}
