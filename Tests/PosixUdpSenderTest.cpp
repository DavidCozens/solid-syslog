#include "CppUTest/TestHarness.h"
#include "PosixUdpSender.h"
#include "SocketSpy.h"
#include <array>
#include <cstring>
#include <netinet/in.h>

// clang-format off
TEST_GROUP(PosixUdpSender)
{
    struct PosixUdpSender_Config config = {0};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslog_Sender* sender = nullptr;

    void setup() override
    {
        SocketSpy_Reset();
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = PosixUdpSender_Create(&config);
    }

    void teardown() override
    {
        PosixUdpSender_Destroy(sender);
    }
};
// clang-format on

TEST(PosixUdpSender, CreateDestroyWorksWithoutCrashing)
{
}

TEST(PosixUdpSender, SingleSendResultsInOneSendtoCall)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

TEST(PosixUdpSender, SendtoReceivesBuffer)
{
    sender->Send(sender, "hello", 5);
    STRCMP_EQUAL("hello", SocketSpy_LastBufAsString());
}

TEST(PosixUdpSender, SendtoCalledWithDefaultPort)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(514, SocketSpy_LastPort());
}

TEST(PosixUdpSender, MaxMessageSizeTransmittedWithoutTruncation)
{
    std::array<char, 1024> buffer{};
    buffer.fill('A');
    sender->Send(sender, buffer.data(), buffer.size());
    LONGS_EQUAL(1024, SocketSpy_LastLen());
    MEMCMP_EQUAL(buffer.data(), SocketSpy_LastBuf(), 1024);
}

TEST(PosixUdpSender, SendtoCalledWithFlagsZero)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(0, SocketSpy_LastFlags());
}

TEST(PosixUdpSender, SendtoCalledWithAddressFamilyAF_INET)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(AF_INET, SocketSpy_LastAddrFamily());
}

TEST(PosixUdpSender, SendtoCalledWithDefaultHost)
{
    sender->Send(sender, "hello", 5);
    STRCMP_EQUAL("127.0.0.1", SocketSpy_LastAddrAsString());
}

TEST(PosixUdpSender, SendtoCalledWithAddrlenOfSockaddrIn)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(sizeof(struct sockaddr_in), SocketSpy_LastAddrLen());
}

TEST(PosixUdpSender, SocketCalledOnCreate)
{
    LONGS_EQUAL(1, SocketSpy_SocketCallCount());
}

TEST(PosixUdpSender, SocketCalledWithAF_INET)
{
    LONGS_EQUAL(AF_INET, SocketSpy_SocketDomain());
}

TEST(PosixUdpSender, SocketCalledWithSOCK_DGRAM)
{
    LONGS_EQUAL(SOCK_DGRAM, SocketSpy_SocketType());
}

TEST(PosixUdpSender, SendtoCalledWithSocketFd)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(SocketSpy_SocketFd(), SocketSpy_LastSendtoFd());
}

TEST(PosixUdpSender, CloseCalledOnDestroy)
{
    PosixUdpSender_Destroy(sender);
    sender = nullptr;
    LONGS_EQUAL(1, SocketSpy_CloseCallCount());
}

TEST(PosixUdpSender, CloseCalledWithSocketFd)
{
    PosixUdpSender_Destroy(sender);
    sender = nullptr;
    LONGS_EQUAL(SocketSpy_SocketFd(), SocketSpy_LastClosedFd());
}

// clang-format off
// Test list — S2.1: Walking Skeleton — PosixUdpSender transmits a buffer
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
// E — Exceptions (deferred — error handling phase)
//   [ ] Create with NULL config returns NULL
//   [ ] Create with valid config returns non-NULL sender
//   [ ] Send called with NULL buffer does nothing, does not crash
//   [ ] Send called with zero length does nothing, does not crash
//   [ ] socket returning -1 handled gracefully — Create returns NULL or Send is a no-op
//   [ ] Unreachable host does not crash
//
// S — Simple scenario
//   [ ] socket called once with AF_INET and SOCK_DGRAM, sendto called once with correct
//       address family and port, close called once on Destroy
// clang-format on
