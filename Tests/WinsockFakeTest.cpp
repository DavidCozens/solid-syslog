#include "CppUTest/TestHarness.h"
#include "WinsockFake.h"
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

// clang-format off
static const char* const TEST_MESSAGE     = "hello";
static const int         TEST_MESSAGE_LEN = 5;
static const char* const TEST_HOST        = "127.0.0.1";
static const int         TEST_PORT        = 514;

TEST_GROUP(WinsockFake)
{
    struct sockaddr_in destination{};

    void setup() override
    {
        WinsockFake_Reset();
        destination.sin_family = AF_INET;
        destination.sin_port   = htons(TEST_PORT);
        inet_pton(AF_INET, TEST_HOST, &destination.sin_addr);
    }
};
// clang-format on

TEST(WinsockFake, SocketRecordsCall)
{
    WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    LONGS_EQUAL(1, WinsockFake_SocketCallCount());
    LONGS_EQUAL(AF_INET, WinsockFake_SocketDomain());
    LONGS_EQUAL(SOCK_DGRAM, WinsockFake_SocketType());
}

TEST(WinsockFake, SocketReturnsInvalidSocketWhenFailing)
{
    WinsockFake_SetSocketFails(true);
    SOCKET fd = WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(fd == INVALID_SOCKET);
}

TEST(WinsockFake, SocketReturnsValidHandleOnSuccess)
{
    SOCKET fd = WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(fd != INVALID_SOCKET);
}

TEST(WinsockFake, SendtoRecordsBufferAndAddress)
{
    SOCKET fd = WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    WinsockFake_sendto(fd, TEST_MESSAGE, TEST_MESSAGE_LEN, 0, (const struct sockaddr*) &destination, sizeof(destination));
    LONGS_EQUAL(1, WinsockFake_SendtoCallCount());
    STRCMP_EQUAL(TEST_MESSAGE, WinsockFake_LastBufAsString());
    LONGS_EQUAL(TEST_MESSAGE_LEN, (int) WinsockFake_LastLen());
    LONGS_EQUAL(TEST_PORT, WinsockFake_LastPort());
    STRCMP_EQUAL(TEST_HOST, WinsockFake_LastAddrAsString());
    LONGS_EQUAL((int) sizeof(destination), WinsockFake_LastAddrLen());
}

TEST(WinsockFake, SendtoReturnsSocketErrorWhenFailing)
{
    WinsockFake_SetSendtoFails(true);
    int result = WinsockFake_sendto(INVALID_SOCKET, TEST_MESSAGE, TEST_MESSAGE_LEN, 0,
                                    (const struct sockaddr*) &destination, sizeof(destination));
    LONGS_EQUAL(SOCKET_ERROR, result);
}

TEST(WinsockFake, SendtoReturnsLengthOnSuccess)
{
    int result = WinsockFake_sendto(INVALID_SOCKET, TEST_MESSAGE, TEST_MESSAGE_LEN, 0,
                                    (const struct sockaddr*) &destination, sizeof(destination));
    LONGS_EQUAL(TEST_MESSAGE_LEN, result);
}

TEST(WinsockFake, ClosesocketRecordsCall)
{
    SOCKET fd = WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    WinsockFake_closesocket(fd);
    LONGS_EQUAL(1, WinsockFake_CloseCallCount());
    CHECK(WinsockFake_LastClosedFd() == fd);
}

TEST(WinsockFake, GetAddrInfoRecordsHostnameAndSocktype)
{
    struct addrinfo  hints = {0, 0, 0, 0, 0, nullptr, nullptr, nullptr};
    hints.ai_socktype      = SOCK_DGRAM;
    struct addrinfo* res   = nullptr;
    WinsockFake_getaddrinfo(TEST_HOST, nullptr, &hints, &res);
    LONGS_EQUAL(1, WinsockFake_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_HOST, WinsockFake_LastGetAddrInfoHostname());
    LONGS_EQUAL(SOCK_DGRAM, WinsockFake_LastGetAddrInfoSocktype());
    CHECK(res != nullptr);
}

TEST(WinsockFake, GetAddrInfoReturnsFailureCode)
{
    WinsockFake_SetGetAddrInfoFails(true);
    struct addrinfo* res = nullptr;
    int              rc  = WinsockFake_getaddrinfo(TEST_HOST, nullptr, nullptr, &res);
    LONGS_EQUAL(EAI_FAIL, rc);
}

TEST(WinsockFake, FreeAddrInfoRecordsCall)
{
    struct addrinfo* res = nullptr;
    WinsockFake_getaddrinfo(TEST_HOST, nullptr, nullptr, &res);
    WinsockFake_freeaddrinfo(res);
    LONGS_EQUAL(1, WinsockFake_FreeAddrInfoCallCount());
}

TEST(WinsockFake, ResetClearsCounters)
{
    WinsockFake_socket(AF_INET, SOCK_DGRAM, 0);
    WinsockFake_Reset();
    LONGS_EQUAL(0, WinsockFake_SocketCallCount());
    LONGS_EQUAL(0, WinsockFake_SendtoCallCount());
    LONGS_EQUAL(0, WinsockFake_CloseCallCount());
    LONGS_EQUAL(0, WinsockFake_GetAddrInfoCallCount());
    LONGS_EQUAL(0, WinsockFake_FreeAddrInfoCallCount());
}
