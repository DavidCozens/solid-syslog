#include "CppUTest/TestHarness.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "SocketFake.h"
#include <arpa/inet.h>
#include <cstdint>
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
    SolidSyslogAddressStorage   resultStorage{};

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create(GetHost, GetPort);
    }

    void teardown() override
    {
        SolidSyslogGetAddrInfoResolver_Destroy();
    }

    bool Resolve(enum SolidSyslogTransport transport = SOLIDSYSLOG_TRANSPORT_UDP)
    {
        struct SolidSyslogAddress* address = SolidSyslogAddress_FromStorage(&resultStorage);
        return resolver->Resolve(resolver, transport, address);
    }

    // Test-only peek: the resolver fills the storage per the POSIX sockaddr_in layout.
    // NOLINTNEXTLINE(modernize-use-nodiscard) -- used through the accessor syntax in tests
    const struct sockaddr_in* Result() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- char-type aliasing, legal and necessary
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(&resultStorage);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) -- reinterpret to platform layout, storage is intptr_t-aligned
        return reinterpret_cast<const struct sockaddr_in*>(bytes);
    }
};

// clang-format on

TEST(SolidSyslogGetAddrInfoResolver, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesAddressFamily)
{
    Resolve();
    LONGS_EQUAL(AF_INET, Result()->sin_family);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesResolvedAddress)
{
    Resolve();
    char addrString[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &Result()->sin_addr, addrString, sizeof(addrString));
    STRCMP_EQUAL(TEST_HOST, addrString);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesPort)
{
    Resolve();
    LONGS_EQUAL(TEST_PORT, ntohs(Result()->sin_port));
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
    LONGS_EQUAL(TEST_ALTERNATE_PORT, ntohs(Result()->sin_port));
}

TEST(SolidSyslogGetAddrInfoResolver, UdpTransportPassesDatagramSocktype)
{
    Resolve(SOLIDSYSLOG_TRANSPORT_UDP);
    LONGS_EQUAL(SOCK_DGRAM, SocketFake_LastGetAddrInfoSocktype());
}

TEST(SolidSyslogGetAddrInfoResolver, TcpTransportPassesStreamSocktype)
{
    Resolve(SOLIDSYSLOG_TRANSPORT_TCP);
    LONGS_EQUAL(SOCK_STREAM, SocketFake_LastGetAddrInfoSocktype());
}

TEST(SolidSyslogGetAddrInfoResolver, ResolveReturnsFalseWhenGetAddrInfoFails)
{
    SocketFake_SetGetAddrInfoFails(true);
    CHECK_FALSE(Resolve());
}

TEST(SolidSyslogGetAddrInfoResolver, ResolveReturnsTrueOnSuccess)
{
    CHECK_TRUE(Resolve());
}

TEST(SolidSyslogGetAddrInfoResolver, ResolveDoesNotFreeAddrInfoWhenGetAddrInfoFails)
{
    SocketFake_SetGetAddrInfoFails(true);
    Resolve();
    LONGS_EQUAL(0, SocketFake_FreeAddrInfoCallCount());
}

TEST(SolidSyslogGetAddrInfoResolver, ResolveFreesAddrInfoOnSuccess)
{
    Resolve();
    LONGS_EQUAL(1, SocketFake_FreeAddrInfoCallCount());
}
