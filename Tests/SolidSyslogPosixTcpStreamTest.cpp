#include "CppUTest/TestHarness.h"
#include "SolidSyslogPosixTcpStream.h"
#include "SolidSyslogStream.h"
#include "SocketFake.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

// clang-format off
static const char* const TEST_MESSAGE     = "hello";
static const size_t      TEST_MESSAGE_LEN = 5;
static const char* const TEST_ADDRESS     = "127.0.0.1";
static const int         TEST_PORT        = 514;

TEST_GROUP(SolidSyslogPosixTcpStream)
{
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogStream* stream = nullptr;
    struct sockaddr_in        addr{};

    void setup() override
    {
        SocketFake_Reset();
        // cppcheck-suppress unreadVariable -- used in tests; cppcheck does not model CppUTest macros
        stream = SolidSyslogPosixTcpStream_Create();
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(TEST_PORT);
        inet_pton(AF_INET, TEST_ADDRESS, &addr.sin_addr);
    }

    void teardown() override
    {
        SolidSyslogPosixTcpStream_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogPosixTcpStream, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogPosixTcpStream, OpenCallsSocketOnce)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(1, SocketFake_SocketCallCount());
}

TEST(SolidSyslogPosixTcpStream, OpenCallsSocketWithAF_INET)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(AF_INET, SocketFake_SocketDomain());
}

TEST(SolidSyslogPosixTcpStream, OpenCallsSocketWithSOCK_STREAM)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(SOCK_STREAM, SocketFake_SocketType());
}

TEST(SolidSyslogPosixTcpStream, OpenEnablesTcpNoDelay)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(1, SocketFake_SetSockOptCallCount());
    LONGS_EQUAL(IPPROTO_TCP, SocketFake_LastSetSockOptLevel());
    LONGS_EQUAL(TCP_NODELAY, SocketFake_LastSetSockOptOptname());
}

TEST(SolidSyslogPosixTcpStream, OpenCallsConnectWithSocketFd)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(1, SocketFake_ConnectCallCount());
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastConnectFd());
}

TEST(SolidSyslogPosixTcpStream, OpenCallsConnectWithProvidedAddress)
{
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(TEST_PORT, SocketFake_LastConnectPort());
    STRCMP_EQUAL(TEST_ADDRESS, SocketFake_LastConnectAddrAsString());
}

TEST(SolidSyslogPosixTcpStream, OpenReturnsTrueOnSuccess)
{
    CHECK_TRUE(SolidSyslogStream_Open(stream, &addr));
}

TEST(SolidSyslogPosixTcpStream, OpenReturnsFalseOnConnectFailure)
{
    SocketFake_SetConnectFails(true);
    CHECK_FALSE(SolidSyslogStream_Open(stream, &addr));
}

TEST(SolidSyslogPosixTcpStream, OpenClosesSocketOnConnectFailure)
{
    SocketFake_SetConnectFails(true);
    SolidSyslogStream_Open(stream, &addr);
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastClosedFd());
}

TEST(SolidSyslogPosixTcpStream, SendCallsSendOnce)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(1, SocketFake_SendCallCount());
}

TEST(SolidSyslogPosixTcpStream, SendPassesBuffer)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    STRCMP_EQUAL(TEST_MESSAGE, SocketFake_SendBufAsString(0));
}

TEST(SolidSyslogPosixTcpStream, SendPassesLength)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(TEST_MESSAGE_LEN, SocketFake_SendLen(0));
}

TEST(SolidSyslogPosixTcpStream, SendPassesMSG_NOSIGNALFlag)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(MSG_NOSIGNAL, SocketFake_SendFlags(0));
}

TEST(SolidSyslogPosixTcpStream, SendPassesSocketFd)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN);
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastSendFd());
}

TEST(SolidSyslogPosixTcpStream, SendReturnsTrueOnSuccess)
{
    SolidSyslogStream_Open(stream, &addr);
    CHECK_TRUE(SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogPosixTcpStream, SendReturnsFalseOnSendFailure)
{
    SolidSyslogStream_Open(stream, &addr);
    SocketFake_SetSendFails(true);
    CHECK_FALSE(SolidSyslogStream_Send(stream, TEST_MESSAGE, TEST_MESSAGE_LEN));
}

TEST(SolidSyslogPosixTcpStream, CloseCallsCloseOnce)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(1, SocketFake_CloseCallCount());
}

TEST(SolidSyslogPosixTcpStream, CloseCalledWithSocketFd)
{
    SolidSyslogStream_Open(stream, &addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(SocketFake_SocketFd(), SocketFake_LastClosedFd());
}

TEST(SolidSyslogPosixTcpStream, CloseIsNoOpWhenNotOpen)
{
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(0, SocketFake_CloseCallCount());
}
