#include "CppUTest/TestHarness.h"
#include "SolidSyslogDatagram.h"
#include "SolidSyslogPosixDatagram.h"
#include "SocketFake.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

// clang-format off
static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;
static const char* const TEST_ADDRESS     = "127.0.0.1";
static const int         TEST_PORT        = 514;

TEST_GROUP(SolidSyslogPosixDatagram)
{
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogDatagram* datagram = nullptr;
    struct sockaddr_in          addr{};

    void setup() override
    {
        SocketFake_Reset();
        // cppcheck-suppress unreadVariable -- used in tests; cppcheck does not model CppUTest macros
        datagram = SolidSyslogPosixDatagram_Create();
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(TEST_PORT);
        inet_pton(AF_INET, TEST_ADDRESS, &addr.sin_addr);
    }

    void teardown() override
    {
        SolidSyslogPosixDatagram_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogPosixDatagram, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogPosixDatagram, OpenCallsSocketOnce)
{
    SolidSyslogDatagram_Open(datagram);
    LONGS_EQUAL(1, SocketFake_SocketCallCount());
}

TEST(SolidSyslogPosixDatagram, OpenCallsSocketWithAF_INET)
{
    SolidSyslogDatagram_Open(datagram);
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
}

TEST(SolidSyslogPosixDatagram, OpenCallsSocketWithSOCK_DGRAM)
{
    SolidSyslogDatagram_Open(datagram);
    LONGS_EQUAL(SOCK_DGRAM, SocketFake_SocketType());
}

TEST(SolidSyslogPosixDatagram, SendToCallsSendtoOnce)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(1, SocketFake_SendtoCallCount());
}

TEST(SolidSyslogPosixDatagram, SendToPassesBuffer)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    STRCMP_EQUAL(TEST_MESSAGE, SocketFake_LastBufAsString());
}

TEST(SolidSyslogPosixDatagram, SendToPassesLength)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(TEST_MESSAGE_LEN, SocketFake_LastLen());
}

TEST(SolidSyslogPosixDatagram, SendToPassesFlagsZero)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(0, SocketFake_LastFlags());
}

TEST(SolidSyslogPosixDatagram, SendToPassesSocketFd)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastSendtoFd());
}

TEST(SolidSyslogPosixDatagram, SendToPassesAddressPort)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(TEST_PORT, SocketFake_LastPort());
}

TEST(SolidSyslogPosixDatagram, SendToPassesAddressHost)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    STRCMP_EQUAL(TEST_ADDRESS, SocketFake_LastAddrAsString());
}

TEST(SolidSyslogPosixDatagram, SendToPassesAddrlenOfSockaddrIn)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr);
    LONGS_EQUAL(sizeof(struct sockaddr_in), SocketFake_LastAddrLen());
}

TEST(SolidSyslogPosixDatagram, SendToReturnsTrueOnSuccess)
{
    SolidSyslogDatagram_Open(datagram);
    CHECK_TRUE(SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr));
}

TEST(SolidSyslogPosixDatagram, SendToReturnsFalseOnSendtoFailure)
{
    SolidSyslogDatagram_Open(datagram);
    SocketFake_SetSendtoFails(true);
    CHECK_FALSE(SolidSyslogDatagram_SendTo(datagram, TEST_MESSAGE, TEST_MESSAGE_LEN, &addr));
}

TEST(SolidSyslogPosixDatagram, CloseCallsCloseOnce)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_Close(datagram);
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogPosixDatagram, CloseCalledWithSocketFd)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_Close(datagram);
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastClosedFd());
}
