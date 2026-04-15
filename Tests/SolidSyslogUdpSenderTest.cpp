#include "CppUTest/TestHarness.h"
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSender.h"
#include "SocketFake.h"
#include <array>
#include <cstring>
#include <netinet/in.h>

// clang-format off
static const char* const TEST_MESSAGE          = "hello";
static const size_t      TEST_MESSAGE_LEN      = 5;
static const char* const TEST_DEFAULT_HOST     = "127.0.0.1";
static const int         TEST_DEFAULT_PORT     = 514;
static const int         TEST_ALTERNATE_PORT   = 9999;
static const size_t      TEST_MAX_MESSAGE_SIZE = 1024;
// clang-format on

static int GetDefaultPort()
{
    return TEST_DEFAULT_PORT;
}

static int GetAlternatePort()
{
    return TEST_ALTERNATE_PORT;
}

static const char* GetDefaultHost()
{
    return TEST_DEFAULT_HOST;
}

static int getHostCallCount;

static const char* SpyGetHost()
{
    getHostCallCount++;
    return TEST_DEFAULT_HOST;
}

static int getPortCallCount;

static int SpyGetPort()
{
    getPortCallCount++;
    return TEST_DEFAULT_PORT;
}

// clang-format off
TEST_GROUP(SolidSyslogUdpSender)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct SolidSyslogUdpSenderConfig config = {};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create();
        config = {resolver, GetDefaultPort, GetDefaultHost};
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogUdpSender_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogUdpSender_Destroy();
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void Send() const
    {
        SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }

    void Send(const void* buffer, size_t size) const
    {
        SolidSyslogSender_Send(sender, buffer, size);
    }
};

// clang-format on

TEST(SolidSyslogUdpSender, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogUdpSender, SendReturnsTrueOnSuccess)
{
    CHECK_TRUE(SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogUdpSender, SendReturnsFalseOnSendtoFailure)
{
    SocketFake_SetSendtoFails(true);
    CHECK_FALSE(SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogUdpSender, SingleSendResultsInOneSendtoCall)
{
    Send();
    LONGS_EQUAL(1, SocketFake_SendtoCallCount());
}

TEST(SolidSyslogUdpSender, SendtoReceivesBuffer)
{
    Send();
    STRCMP_EQUAL(TEST_MESSAGE, SocketFake_LastBufAsString());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithDefaultPort)
{
    Send();
    LONGS_EQUAL(TEST_DEFAULT_PORT, SocketFake_LastPort());
}

TEST(SolidSyslogUdpSender, MaxMessageSizeTransmittedWithoutTruncation)
{
    std::array<char, TEST_MAX_MESSAGE_SIZE> buffer{};
    buffer.fill('A');
    Send(buffer.data(), buffer.size());
    LONGS_EQUAL(TEST_MAX_MESSAGE_SIZE, SocketFake_LastLen());
    MEMCMP_EQUAL(buffer.data(), SocketFake_LastBuf(), TEST_MAX_MESSAGE_SIZE);
}

TEST(SolidSyslogUdpSender, SendtoCalledWithFlagsZero)
{
    Send();
    LONGS_EQUAL(0, SocketFake_LastFlags());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithAddressFamilyAF_INET)
{
    Send();
    LONGS_EQUAL(AF_INET, SocketFake_LastAddrFamily());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithDefaultHost)
{
    Send();
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketFake_LastAddrAsString());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithAddrlenOfSockaddrIn)
{
    Send();
    LONGS_EQUAL(sizeof(struct sockaddr_in), SocketFake_LastAddrLen());
}

TEST(SolidSyslogUdpSender, SocketCalledOnCreate)
{
    LONGS_EQUAL(1, SocketFake_SocketCallCount());
}

TEST(SolidSyslogUdpSender, SocketCalledWithAF_INET)
{
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
}

TEST(SolidSyslogUdpSender, SocketCalledWithSOCK_DGRAM)
{
    LONGS_EQUAL(SOCK_DGRAM, SocketFake_SocketType());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithSocketFd)
{
    Send();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastSendtoFd());
}

IGNORE_TEST(SolidSyslogUdpSender, HappyPathOnly)
{
    // Error handling not yet implemented — see Epic #31
    //   Create with NULL config returns NULL
    //   Create with valid config returns non-NULL sender
    //   Destroy with NULL sender does nothing, does not crash
    //   Send called with NULL buffer does nothing, does not crash
    //   Send called with zero length does nothing, does not crash
    //   socket() returning -1 handled gracefully — Create returns NULL or Send is a no-op
    //   Unreachable host does not crash
    //
    // Address resolution strategy (getaddrinfo vs inet_pton, malloc policy, ADR) — see Story #34
}

// Destroy tests manage their own sender lifetime — no teardown Destroy needed.
// clang-format off
TEST_GROUP(SolidSyslogUdpSenderDestroy)
{
    struct SolidSyslogResolver* resolver = nullptr;
    // cppcheck-suppress unreadVariable -- used in test bodies; cppcheck does not model CppUTest macros
    struct SolidSyslogUdpSenderConfig config = {};

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create();
        config = {resolver, GetDefaultPort, GetDefaultHost};
    }

    void teardown() override
    {
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void CreateAndDestroy() const
    {
        SolidSyslogUdpSender_Create(&config);
        SolidSyslogUdpSender_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogUdpSenderDestroy, CloseCalledOnDestroy)
{
    CreateAndDestroy();
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogUdpSenderDestroy, CloseCalledWithSocketFd)
{
    CreateAndDestroy();
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastClosedFd());
}

TEST(SolidSyslogUdpSenderDestroy, SimpleScenario)
{
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&config);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    SolidSyslogUdpSender_Destroy();

    LONGS_EQUAL(1, SocketFake_SocketCallCount());
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
    LONGS_EQUAL(SOCK_DGRAM, SocketFake_SocketType());
    LONGS_EQUAL(1, SocketFake_SendtoCallCount());
    LONGS_EQUAL(AF_INET, SocketFake_LastAddrFamily());
    LONGS_EQUAL(TEST_DEFAULT_PORT, SocketFake_LastPort());
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

// clang-format off
TEST_GROUP(SolidSyslogUdpSenderConfig)
{
    struct SolidSyslogResolver* resolver = nullptr;
    // cppcheck-suppress unreadVariable -- assigned in CreateSender and used via SolidSyslogUdpSender_Create; cppcheck does not model CppUTest macros
    struct SolidSyslogUdpSenderConfig config = {};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        getPortCallCount = 0;
        getHostCallCount = 0;
        resolver = SolidSyslogGetAddrInfoResolver_Create();
        config = {resolver, GetDefaultPort, GetDefaultHost};
    }

    void teardown() override
    {
        SolidSyslogUdpSender_Destroy();
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void CreateSender()
    {
        sender = SolidSyslogUdpSender_Create(&config);
    }

    void Send() const
    {
        SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogUdpSenderConfig, GetPortCalledOnCreate)
{
    config.getPort = SpyGetPort;
    CreateSender();
    LONGS_EQUAL(1, getPortCallCount);
    Send();
    LONGS_EQUAL(1, getPortCallCount);
}

TEST(SolidSyslogUdpSenderConfig, SendtoCalledWithConfiguredPort)
{
    config.getPort = GetAlternatePort;
    CreateSender();
    Send();
    LONGS_EQUAL(TEST_ALTERNATE_PORT, SocketFake_LastPort());
}

TEST(SolidSyslogUdpSenderConfig, GetHostCalledOnCreate)
{
    config.getHost = SpyGetHost;
    CreateSender();
    LONGS_EQUAL(1, getHostCallCount);
    Send();
    LONGS_EQUAL(1, getHostCallCount);
}

TEST(SolidSyslogUdpSenderConfig, GetAddrInfoCalledWithHostnameFromGetHost)
{
    config.getHost = SpyGetHost;
    CreateSender();
    LONGS_EQUAL(1, SocketFake_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketFake_LastGetAddrInfoHostname());
}

TEST(SolidSyslogUdpSenderConfig, SendtoCalledWithResolvedAddress)
{
    CreateSender();
    Send();
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketFake_LastAddrAsString());
}
