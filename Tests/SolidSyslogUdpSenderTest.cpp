#include "CppUTest/TestHarness.h"
#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSender.h"
#include "SocketSpy.h"
#include <array>
#include <cstring>
#include <netinet/in.h>

// clang-format off
static const char * const TEST_MESSAGE          = "hello";
static const size_t       TEST_MESSAGE_LEN      = 5;
static const char * const TEST_DEFAULT_HOST     = "127.0.0.1";
static const int          TEST_DEFAULT_PORT     = 514;
static const int          TEST_ALTERNATE_PORT   = 9999;
static const size_t       TEST_MAX_MESSAGE_SIZE = 1024;
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
    struct SolidSyslogUdpSenderConfig config = {GetDefaultPort, GetDefaultHost};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketSpy_Reset();
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = SolidSyslogUdpSender_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogUdpSender_Destroy(sender);
    }

    void Send() const
    {
        SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    }
};

// clang-format on

TEST(SolidSyslogUdpSender, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogUdpSender, SingleSendResultsInOneSendtoCall)
{
    Send();
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

TEST(SolidSyslogUdpSender, SendtoReceivesBuffer)
{
    Send();
    STRCMP_EQUAL(TEST_MESSAGE, SocketSpy_LastBufAsString());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithDefaultPort)
{
    Send();
    LONGS_EQUAL(TEST_DEFAULT_PORT, SocketSpy_LastPort());
}

TEST(SolidSyslogUdpSender, MaxMessageSizeTransmittedWithoutTruncation)
{
    std::array<char, TEST_MAX_MESSAGE_SIZE> buffer{};
    buffer.fill('A');
    SolidSyslogSender_Send(sender, buffer.data(), buffer.size());
    LONGS_EQUAL(TEST_MAX_MESSAGE_SIZE, SocketSpy_LastLen());
    MEMCMP_EQUAL(buffer.data(), SocketSpy_LastBuf(), TEST_MAX_MESSAGE_SIZE);
}

TEST(SolidSyslogUdpSender, SendtoCalledWithFlagsZero)
{
    Send();
    LONGS_EQUAL(0, SocketSpy_LastFlags());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithAddressFamilyAF_INET)
{
    Send();
    LONGS_EQUAL(AF_INET, SocketSpy_LastAddrFamily());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithDefaultHost)
{
    Send();
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketSpy_LastAddrAsString());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithAddrlenOfSockaddrIn)
{
    Send();
    LONGS_EQUAL(sizeof(struct sockaddr_in), SocketSpy_LastAddrLen());
}

TEST(SolidSyslogUdpSender, SocketCalledOnCreate)
{
    LONGS_EQUAL(1, SocketSpy_SocketCallCount());
}

TEST(SolidSyslogUdpSender, SocketCalledWithAF_INET)
{
    LONGS_EQUAL(AF_INET, SocketSpy_SocketDomain());
}

TEST(SolidSyslogUdpSender, SocketCalledWithSOCK_DGRAM)
{
    LONGS_EQUAL(SOCK_DGRAM, SocketSpy_SocketType());
}

TEST(SolidSyslogUdpSender, SendtoCalledWithSocketFd)
{
    Send();
    LONGS_EQUAL(SocketSpy_SocketFd(), SocketSpy_LastSendtoFd());
}

// Destroy tests manage their own sender lifetime — no teardown Destroy needed.
// clang-format off
TEST_GROUP(SolidSyslogUdpSenderDestroy)
{
    // cppcheck-suppress unreadVariable -- used in test bodies; cppcheck does not model CppUTest macros
    struct SolidSyslogUdpSenderConfig config = {GetDefaultPort, GetDefaultHost};

    void setup() override { SocketSpy_Reset(); }
    void teardown() override {}

    void CreateAndDestroy() const
    {
        struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&config);
        SolidSyslogUdpSender_Destroy(sender);
    }
};

// clang-format on

TEST(SolidSyslogUdpSenderDestroy, CloseCalledOnDestroy)
{
    CreateAndDestroy();
    LONGS_EQUAL(1, SocketSpy_CloseCallCount());
}

TEST(SolidSyslogUdpSenderDestroy, CloseCalledWithSocketFd)
{
    CreateAndDestroy();
    LONGS_EQUAL(SocketSpy_SocketFd(), SocketSpy_LastClosedFd());
}

TEST(SolidSyslogUdpSenderDestroy, SimpleScenario)
{
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&config);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    SolidSyslogUdpSender_Destroy(sender);

    LONGS_EQUAL(1, SocketSpy_SocketCallCount());
    LONGS_EQUAL(AF_INET, SocketSpy_SocketDomain());
    LONGS_EQUAL(SOCK_DGRAM, SocketSpy_SocketType());
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
    LONGS_EQUAL(AF_INET, SocketSpy_LastAddrFamily());
    LONGS_EQUAL(TEST_DEFAULT_PORT, SocketSpy_LastPort());
    LONGS_EQUAL(1, SocketSpy_CloseCallCount());
}

// clang-format off
TEST_GROUP(SolidSyslogUdpSenderConfig)
{
    // cppcheck-suppress unreadVariable -- assigned in CreateSender and used via SolidSyslogUdpSender_Create; cppcheck does not model CppUTest macros
    struct SolidSyslogUdpSenderConfig config = {};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogSender* sender = nullptr;

    void setup() override
    {
        SocketSpy_Reset();
        getPortCallCount = 0;
        getHostCallCount = 0;
    }

    void teardown() override
    {
        SolidSyslogUdpSender_Destroy(sender);
    }

    void CreateSender(int (*getPort)(), const char* (*getHost)())
    {
        config = {getPort, getHost};
        sender = SolidSyslogUdpSender_Create(&config);
    }
};

// clang-format on

TEST(SolidSyslogUdpSenderConfig, GetPortCalledOnCreate)
{
    CreateSender(SpyGetPort, GetDefaultHost);
    LONGS_EQUAL(1, getPortCallCount);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(1, getPortCallCount);
}

TEST(SolidSyslogUdpSenderConfig, SendtoCalledWithConfiguredPort)
{
    CreateSender(GetAlternatePort, GetDefaultHost);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(TEST_ALTERNATE_PORT, SocketSpy_LastPort());
}

TEST(SolidSyslogUdpSenderConfig, GetHostCalledOnCreate)
{
    CreateSender(GetDefaultPort, SpyGetHost);
    LONGS_EQUAL(1, getHostCallCount);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(1, getHostCallCount);
}

TEST(SolidSyslogUdpSenderConfig, GetAddrInfoCalledWithHostnameFromGetHost)
{
    CreateSender(GetDefaultPort, SpyGetHost);
    LONGS_EQUAL(1, SocketSpy_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketSpy_LastGetAddrInfoHostname());
}

TEST(SolidSyslogUdpSenderConfig, SendtoCalledWithResolvedAddress)
{
    CreateSender(GetDefaultPort, GetDefaultHost);
    SolidSyslogSender_Send(sender, TEST_MESSAGE, TEST_MESSAGE_LEN);
    STRCMP_EQUAL(TEST_DEFAULT_HOST, SocketSpy_LastAddrAsString());
}

// clang-format off
// Test list — S2.1/S2.2: SolidSyslogUdpSender transmits a buffer with injected config
//
// Z — Zero
//   [x] CreateDestroyWorksWithoutCrashing
//
// O — One
//   [x] A single Send call results in exactly one call to SocketSpy sendto
//   [x] SocketSpy sendto called with correct buffer contents
//   [x] SocketSpy sendto called with correct destination port (default 514)
//
// B — Boundaries
//   [x] Maximum RFC 5426 recommended message size (1024 bytes) transmitted
//       without truncation — length and content verified
//
// sendto argument defaults
//   [x] sendto called with flags = 0
//   [x] sendto called with address family AF_INET
//   [x] sendto called with default host 127.0.0.1
//   [x] sendto called with addrlen = sizeof(struct sockaddr_in)
//   [x] sendto called with fd returned by socket()
//
// socket/close lifecycle
//   [x] socket() called once on Create
//   [x] socket() called with AF_INET
//   [x] socket() called with SOCK_DGRAM
//   [x] close() called once on Destroy
//   [x] close() called with fd returned by socket()
//
// Config injection (S2.2)
//   [x] getPort() called once on Create; not called again on Send
//   [x] sendto called with port returned by getPort()
//   [x] getHost() called once on Create; not called again on Send
//   [x] getaddrinfo() called once on Create with hostname from getHost()
//   [x] sendto called with address resolved from getHost()
//
// E — Exceptions (deferred — error handling phase)
//   [ ] Create with NULL config returns NULL
//   [ ] Create with valid config returns non-NULL sender
//   [ ] Destroy with NULL sender does nothing, does not crash
//   [ ] Send called with NULL buffer does nothing, does not crash
//   [ ] Send called with zero length does nothing, does not crash
//   [ ] socket returning -1 handled gracefully — Create returns NULL or Send is a no-op
//   [ ] Unreachable host does not crash
//
// S — Simple scenario
//   [x] socket called once with AF_INET and SOCK_DGRAM, sendto called once with correct
//       address family and port, close called once on Destroy
// clang-format on
