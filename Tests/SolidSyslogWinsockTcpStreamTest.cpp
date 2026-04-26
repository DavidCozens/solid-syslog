#include "CppUTest/TestHarness.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogTransport.h"
#include "SolidSyslogWinsockTcpStream.h"
#include "SolidSyslogWinsockTcpStreamInternal.h"
#include "WinsockFake.h"
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

// clang-format off
static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;
static const char* const TEST_ADDRESS     = "127.0.0.1";
static const int         TEST_PORT        = 514;

TEST_GROUP(SolidSyslogWinsockTcpStream)
{
    SolidSyslogWinsockTcpStreamStorage streamStorage{};
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogStream* stream = nullptr;
    SolidSyslogAddressStorage addrStorage{};
    // cppcheck-suppress unreadVariable -- assigned in setup; cppcheck does not model CppUTest macros
    struct SolidSyslogAddress* addr = nullptr;

    void setup() override
    {
        WinsockFake_Reset();
        UT_PTR_SET(WinsockTcpStream_socket,      WinsockFake_socket);
        UT_PTR_SET(WinsockTcpStream_connect,     WinsockFake_connect);
        UT_PTR_SET(WinsockTcpStream_send,        WinsockFake_send);
        UT_PTR_SET(WinsockTcpStream_recv,        WinsockFake_recv);
        UT_PTR_SET(WinsockTcpStream_setsockopt,  WinsockFake_setsockopt);
        UT_PTR_SET(WinsockTcpStream_closesocket, WinsockFake_closesocket);
        // cppcheck-suppress unreadVariable -- used in tests; cppcheck does not model CppUTest macros
        stream = SolidSyslogWinsockTcpStream_Create(&streamStorage);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- char-type aliasing, legal and necessary
        auto* bytes = reinterpret_cast<std::uint8_t*>(&addrStorage);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- reinterpret to platform layout, storage is intptr_t-aligned
        auto* sin       = reinterpret_cast<struct sockaddr_in*>(bytes);
        sin->sin_family = AF_INET;
        sin->sin_port   = htons(TEST_PORT);
        inet_pton(AF_INET, TEST_ADDRESS, &sin->sin_addr);
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        addr = SolidSyslogAddress_FromStorage(&addrStorage);
    }

    void teardown() override
    {
        SolidSyslogWinsockTcpStream_Destroy(stream);
    }
};

// clang-format on

TEST(SolidSyslogWinsockTcpStream, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogWinsockTcpStream, CreateReturnsHandleInsideCallerSuppliedStorage)
{
    SolidSyslogWinsockTcpStreamStorage storage{};
    struct SolidSyslogStream*          localStream = SolidSyslogWinsockTcpStream_Create(&storage);
    POINTERS_EQUAL(&storage, localStream);
    SolidSyslogWinsockTcpStream_Destroy(localStream);
}

TEST(SolidSyslogWinsockTcpStream, OpenCallsSocketOnce)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, WinsockFake_SocketCallCount());
}

TEST(SolidSyslogWinsockTcpStream, OpenCallsSocketWithAF_INET)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(AF_INET, WinsockFake_SocketDomain());
}

TEST(SolidSyslogWinsockTcpStream, OpenCallsSocketWithSOCK_STREAM)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(SOCK_STREAM, WinsockFake_SocketType());
}

TEST(SolidSyslogWinsockTcpStream, OpenEnablesTcpNoDelay)
{
    SolidSyslogStream_Open(stream, addr);
    CHECK_TRUE(WinsockFake_HasSetSockOpt(IPPROTO_TCP, TCP_NODELAY));
}

TEST(SolidSyslogWinsockTcpStream, OpenSetsSendTimeout)
{
    SolidSyslogStream_Open(stream, addr);
    CHECK_TRUE(WinsockFake_HasSetSockOpt(SOL_SOCKET, SO_SNDTIMEO));
}

TEST(SolidSyslogWinsockTcpStream, OpenCallsConnectWithSocketFd)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, WinsockFake_ConnectCallCount());
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastConnectFd());
}

TEST(SolidSyslogWinsockTcpStream, OpenCallsConnectWithProvidedAddress)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(TEST_PORT, WinsockFake_LastConnectPort());
    STRCMP_EQUAL(TEST_ADDRESS, WinsockFake_LastConnectAddrAsString());
}

TEST(SolidSyslogWinsockTcpStream, OpenReturnsTrueOnSuccess)
{
    CHECK_TRUE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogWinsockTcpStream, OpenReturnsFalseOnConnectFailure)
{
    WinsockFake_SetConnectFails(true);
    CHECK_FALSE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogWinsockTcpStream, OpenReturnsFalseWhenSocketFails)
{
    WinsockFake_SetSocketFails(true);
    CHECK_FALSE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogWinsockTcpStream, OpenSkipsConnectAndSetsockoptWhenSocketFails)
{
    WinsockFake_SetSocketFails(true);
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(0, WinsockFake_ConnectCallCount());
    LONGS_EQUAL(0, WinsockFake_SetSockOptCallCount());
}

TEST(SolidSyslogWinsockTcpStream, OpenClosesSocketOnConnectFailure)
{
    WinsockFake_SetConnectFails(true);
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, WinsockFake_CloseCallCount());
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastClosedFd());
}

TEST(SolidSyslogWinsockTcpStream, SendCallsSendOnce)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(1, WinsockFake_SendCallCount());
}

TEST(SolidSyslogWinsockTcpStream, SendPassesBuffer)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    STRCMP_EQUAL(TEST_MESSAGE, WinsockFake_SendBufAsString(0));
}

TEST(SolidSyslogWinsockTcpStream, SendPassesLength)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(TEST_MESSAGE_LEN, WinsockFake_SendLen(0));
}

TEST(SolidSyslogWinsockTcpStream, SendPassesZeroFlags)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(0, WinsockFake_SendFlags(0));
}

TEST(SolidSyslogWinsockTcpStream, SendPassesSocketFd)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastSendFd());
}

TEST(SolidSyslogWinsockTcpStream, SendReturnsTrueOnSuccess)
{
    SolidSyslogStream_Open(stream, addr);
    CHECK_TRUE(SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogWinsockTcpStream, SendReturnsFalseOnSendFailure)
{
    SolidSyslogStream_Open(stream, addr);
    WinsockFake_SetSendFails(true);
    CHECK_FALSE(SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogWinsockTcpStream, SendReturnsFalseOnShortWrite)
{
    SolidSyslogStream_Open(stream, addr);
    WinsockFake_SetSendReturn(3);
    CHECK_FALSE(SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogWinsockTcpStream, SendDoesNotRetryAfterShortWrite)
{
    SolidSyslogStream_Open(stream, addr);
    WinsockFake_SetSendReturn(3);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(1, WinsockFake_SendCallCount());
}

TEST(SolidSyslogWinsockTcpStream, CloseCallsCloseOnce)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(1, WinsockFake_CloseCallCount());
}

TEST(SolidSyslogWinsockTcpStream, CloseCalledWithSocketFd)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastClosedFd());
}

TEST(SolidSyslogWinsockTcpStream, CloseIsNoOpWhenNotOpen)
{
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(0, WinsockFake_CloseCallCount());
}

TEST(SolidSyslogWinsockTcpStream, ReadCallsRecvOnce)
{
    SolidSyslogStream_Open(stream, addr);
    char buf[16];
    SolidSyslogStream_Read(stream, buf, sizeof(buf));
    LONGS_EQUAL(1, WinsockFake_RecvCallCount());
}

TEST(SolidSyslogWinsockTcpStream, ReadPassesSocketFdToRecv)
{
    SolidSyslogStream_Open(stream, addr);
    char buf[16];
    SolidSyslogStream_Read(stream, buf, sizeof(buf));
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastRecvFd());
}

TEST(SolidSyslogWinsockTcpStream, ReadPassesBufferToRecv)
{
    SolidSyslogStream_Open(stream, addr);
    char buf[16];
    SolidSyslogStream_Read(stream, buf, sizeof(buf));
    POINTERS_EQUAL(buf, WinsockFake_LastRecvBuf());
}

TEST(SolidSyslogWinsockTcpStream, ReadPassesLengthToRecv)
{
    SolidSyslogStream_Open(stream, addr);
    char buf[16];
    SolidSyslogStream_Read(stream, buf, sizeof(buf));
    LONGS_EQUAL(sizeof(buf), WinsockFake_LastRecvLen());
}

TEST(SolidSyslogWinsockTcpStream, ReadPassesZeroFlagsToRecv)
{
    SolidSyslogStream_Open(stream, addr);
    char buf[16];
    SolidSyslogStream_Read(stream, buf, sizeof(buf));
    LONGS_EQUAL(0, WinsockFake_LastRecvFlags());
}

TEST(SolidSyslogWinsockTcpStream, ReadReturnsRecvReturnValue)
{
    WinsockFake_SetRecvReturn(7);
    SolidSyslogStream_Open(stream, addr);
    char             buf[16];
    SolidSyslogSsize n = SolidSyslogStream_Read(stream, buf, sizeof(buf));
    LONGS_EQUAL(7, n);
}

TEST(SolidSyslogWinsockTcpStream, DestroyClosesOpenSocket)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogWinsockTcpStream_Destroy(stream);
    LONGS_EQUAL(1, WinsockFake_CloseCallCount());
}

TEST(SolidSyslogWinsockTcpStream, DestroyClosesWithSocketFd)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogWinsockTcpStream_Destroy(stream);
    CHECK(WinsockFake_SocketFd() == WinsockFake_LastClosedFd());
}

TEST(SolidSyslogWinsockTcpStream, DefaultPortMatchesRfc6587)
{
    LONGS_EQUAL(601, SOLIDSYSLOG_TCP_DEFAULT_PORT);
}
