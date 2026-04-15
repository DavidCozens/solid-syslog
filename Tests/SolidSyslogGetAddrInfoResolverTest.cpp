#include "CppUTest/TestHarness.h"
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "SocketFake.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// clang-format off
static const char* const TEST_HOST           = "127.0.0.1";
static const int         TEST_PORT           = 514;
static const char* const TEST_ALTERNATE_HOST = "192.168.1.1";
static const int         TEST_ALTERNATE_PORT = 9999;
// clang-format on

static const char* GetHost()
{
    return TEST_HOST;
}

static int GetPort()
{
    return TEST_PORT;
}

static const char* GetAlternateHost()
{
    return TEST_ALTERNATE_HOST;
}

static int GetAlternatePort()
{
    return TEST_ALTERNATE_PORT;
}

// clang-format off
TEST_GROUP(SolidSyslogGetAddrInfoResolver)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct sockaddr_in result = {};

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetPort);
    }

    void teardown() override
    {
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    void Resolve()
    {
        resolver->Resolve(resolver, SOLIDSYSLOG_TRANSPORT_UDP, &result);
    }
};

// clang-format on

TEST(SolidSyslogGetAddrInfoResolver, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesAddressFamily)
{
    Resolve();
    LONGS_EQUAL(AF_INET, result.sin_family);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesResolvedAddress)
{
    Resolve();
    char addrString[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &result.sin_addr, addrString, sizeof(addrString));
    STRCMP_EQUAL(TEST_HOST, addrString);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesPort)
{
    Resolve();
    LONGS_EQUAL(TEST_PORT, ntohs(result.sin_port));
}

TEST(SolidSyslogGetAddrInfoResolver, GetAddrInfoCalledWithHostname)
{
    Resolve();
    LONGS_EQUAL(1, SocketFake_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_HOST, SocketFake_LastGetAddrInfoHostname());
}

TEST(SolidSyslogGetAddrInfoResolver, AlternateHostResolvesCorrectly)
{
    resolver = SolidSyslogGetAddrInfoResolver_Create(GetAlternateHost, GetPort);
    Resolve();
    STRCMP_EQUAL(TEST_ALTERNATE_HOST, SocketFake_LastGetAddrInfoHostname());
}

TEST(SolidSyslogGetAddrInfoResolver, AlternatePortResolvesCorrectly)
{
    resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetAlternatePort);
    Resolve();
    LONGS_EQUAL(TEST_ALTERNATE_PORT, ntohs(result.sin_port));
}

TEST(SolidSyslogGetAddrInfoResolver, UdpTransportPassesDatagramSocktype)
{
    resolver->Resolve(resolver, SOLIDSYSLOG_TRANSPORT_UDP, &result);
    LONGS_EQUAL(SOCK_DGRAM, SocketFake_LastGetAddrInfoSocktype());
}

TEST(SolidSyslogGetAddrInfoResolver, TcpTransportPassesStreamSocktype)
{
    resolver->Resolve(resolver, SOLIDSYSLOG_TRANSPORT_TCP, &result);
    LONGS_EQUAL(SOCK_STREAM, SocketFake_LastGetAddrInfoSocktype());
}
