#include "CppUTest/TestHarness.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogResolverDefinition.h"
#include "SolidSyslogWinsockResolver.h"
#include "SolidSyslogWinsockResolverInternal.h"
#include "WinsockFake.h"
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

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
TEST_GROUP(SolidSyslogWinsockResolver)
{
    struct SolidSyslogResolver* resolver = nullptr;
    SolidSyslogAddressStorage   resultStorage{};

    void setup() override
    {
        WinsockFake_Reset();
        UT_PTR_SET(Winsock_getaddrinfo, WinsockFake_getaddrinfo);
        UT_PTR_SET(Winsock_freeaddrinfo, WinsockFake_freeaddrinfo);
        resolver = SolidSyslogWinsockResolver_Create(GetHost, GetPort);
    }

    void teardown() override
    {
        SolidSyslogWinsockResolver_Destroy();
    }

    bool Resolve(enum SolidSyslogTransport transport = SOLIDSYSLOG_TRANSPORT_UDP)
    {
        struct SolidSyslogAddress* address = SolidSyslogAddress_FromStorage(&resultStorage);
        return resolver->Resolve(resolver, transport, address);
    }

    // NOLINTNEXTLINE(modernize-use-nodiscard) -- used through accessor syntax in tests
    const struct sockaddr_in* Result() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- char-type aliasing, legal and necessary
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(&resultStorage);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- reinterpret to platform layout, storage is intptr_t-aligned
        return reinterpret_cast<const struct sockaddr_in*>(bytes);
    }
};

// clang-format on

TEST(SolidSyslogWinsockResolver, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogWinsockResolver, ResolvePopulatesAddressFamily)
{
    Resolve();
    LONGS_EQUAL(AF_INET, Result()->sin_family);
}

TEST(SolidSyslogWinsockResolver, ResolvePopulatesResolvedAddress)
{
    Resolve();
    char addrString[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &Result()->sin_addr, addrString, sizeof(addrString));
    STRCMP_EQUAL(TEST_HOST, addrString);
}

TEST(SolidSyslogWinsockResolver, ResolvePopulatesPort)
{
    Resolve();
    LONGS_EQUAL(TEST_PORT, ntohs(Result()->sin_port));
}

TEST(SolidSyslogWinsockResolver, GetAddrInfoCalledWithHostname)
{
    Resolve();
    LONGS_EQUAL(1, WinsockFake_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_HOST, WinsockFake_LastGetAddrInfoHostname());
}

TEST(SolidSyslogWinsockResolver, AlternateHostResolvesCorrectly)
{
    resolver = SolidSyslogWinsockResolver_Create(GetAlternateHost, GetPort);
    Resolve();
    STRCMP_EQUAL(TEST_ALTERNATE_HOST, WinsockFake_LastGetAddrInfoHostname());
}

TEST(SolidSyslogWinsockResolver, AlternatePortResolvesCorrectly)
{
    resolver = SolidSyslogWinsockResolver_Create(GetHost, GetAlternatePort);
    Resolve();
    LONGS_EQUAL(TEST_ALTERNATE_PORT, ntohs(Result()->sin_port));
}

TEST(SolidSyslogWinsockResolver, UdpTransportPassesDatagramSocktype)
{
    Resolve(SOLIDSYSLOG_TRANSPORT_UDP);
    LONGS_EQUAL(SOCK_DGRAM, WinsockFake_LastGetAddrInfoSocktype());
}

TEST(SolidSyslogWinsockResolver, TcpTransportPassesStreamSocktype)
{
    Resolve(SOLIDSYSLOG_TRANSPORT_TCP);
    LONGS_EQUAL(SOCK_STREAM, WinsockFake_LastGetAddrInfoSocktype());
}

TEST(SolidSyslogWinsockResolver, ResolveReturnsFalseWhenGetAddrInfoFails)
{
    WinsockFake_SetGetAddrInfoFails(true);
    CHECK_FALSE(Resolve());
}

TEST(SolidSyslogWinsockResolver, ResolveReturnsTrueOnSuccess)
{
    CHECK_TRUE(Resolve());
}

TEST(SolidSyslogWinsockResolver, ResolveDoesNotFreeAddrInfoWhenGetAddrInfoFails)
{
    WinsockFake_SetGetAddrInfoFails(true);
    Resolve();
    LONGS_EQUAL(0, WinsockFake_FreeAddrInfoCallCount());
}

TEST(SolidSyslogWinsockResolver, ResolveFreesAddrInfoOnSuccess)
{
    Resolve();
    LONGS_EQUAL(1, WinsockFake_FreeAddrInfoCallCount());
}
