#include "CppUTest/TestHarness.h"
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "SocketFake.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static const char* const TEST_HOST = "127.0.0.1";

// clang-format off
TEST_GROUP(SolidSyslogGetAddrInfoResolver)
{
    struct SolidSyslogResolver* resolver = nullptr;
    struct sockaddr_in result = {};

    void setup() override
    {
        SocketFake_Reset();
        resolver = SolidSyslogGetAddrInfoResolver_Create();
    }

    void teardown() override
    {
        SolidSyslogGetAddrInfoResolver_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogGetAddrInfoResolver, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesAddressFamily)
{
    resolver->Resolve(resolver, TEST_HOST, SOCK_DGRAM, &result);
    LONGS_EQUAL(AF_INET, result.sin_family);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolvePopulatesResolvedAddress)
{
    resolver->Resolve(resolver, TEST_HOST, SOCK_DGRAM, &result);
    char addrString[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &result.sin_addr, addrString, sizeof(addrString));
    STRCMP_EQUAL(TEST_HOST, addrString);
}

TEST(SolidSyslogGetAddrInfoResolver, ResolveSetsPortToZero)
{
    resolver->Resolve(resolver, TEST_HOST, SOCK_DGRAM, &result);
    LONGS_EQUAL(0, ntohs(result.sin_port));
}

TEST(SolidSyslogGetAddrInfoResolver, GetAddrInfoCalledWithHostname)
{
    resolver->Resolve(resolver, TEST_HOST, SOCK_DGRAM, &result);
    LONGS_EQUAL(1, SocketFake_GetAddrInfoCallCount());
    STRCMP_EQUAL(TEST_HOST, SocketFake_LastGetAddrInfoHostname());
}
