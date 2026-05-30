#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting;

#include <cstdint>

#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "SolidSyslogLwipRawAddress.h"
#include "SolidSyslogLwipRawAddressPrivate.h"
#include "SolidSyslogLwipRawDnsResolver.h"
#include "SolidSyslogLwipRawDnsResolverErrors.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "SolidSyslogTransport.h"
#include "SolidSyslogTunables.h"
#include "lwip/ip_addr.h"

// Asserts handle is non-null and not one of the slots in pool.
#define CHECK_IS_FALLBACK(handle, pool)                                                \
    do                                                                                 \
    {                                                                                  \
        CHECK_TEXT((handle) != nullptr, "Fallback handle was nullptr");                \
        for (auto* slot : (pool))                                                      \
        {                                                                              \
            CHECK_TEXT(slot != nullptr, "pool slot was nullptr (FillPool failed?)");   \
            CHECK_TEXT((handle) != slot, "Fallback handle collided with a pool slot"); \
        }                                                                              \
    } while (0)

// Asserts the most recent ErrorHandlerFake call matched (severity, source, code).
#define CHECK_REPORTED(severity, source, code)                     \
    do                                                             \
    {                                                              \
        CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);                \
        LONGS_EQUAL((severity), ErrorHandlerFake_LastSeverity());  \
        POINTERS_EQUAL(&(source), ErrorHandlerFake_LastSource());  \
        UNSIGNED_LONGS_EQUAL((code), ErrorHandlerFake_LastCode()); \
    } while (0)

static const char* const TEST_HOST = "syslog-ng";
static const uint16_t TEST_PORT = 514;

extern "C" void FakeSleep(int milliseconds)
{
    (void) milliseconds;
}

// clang-format off
TEST_GROUP(SolidSyslogLwipRawDnsResolver)
{
    struct SolidSyslogLwipRawDnsResolverConfig config = {};
    struct SolidSyslogResolver* resolver = nullptr;
    struct SolidSyslogAddress*  addr     = nullptr;

    void setup() override
    {
        config.Sleep = FakeSleep;
        resolver     = SolidSyslogLwipRawDnsResolver_Create(&config);
        addr         = SolidSyslogLwipRawAddress_Create();
    }

    void teardown() override
    {
        SolidSyslogLwipRawAddress_Destroy(addr);
        SolidSyslogLwipRawDnsResolver_Destroy(resolver);
    }

    bool Resolve(const char* host = TEST_HOST, uint16_t port = TEST_PORT, enum SolidSyslogTransport transport = SOLIDSYSLOG_TRANSPORT_UDP) const
    {
        return SolidSyslogResolver_Resolve(resolver, transport, host, port, addr);
    }
};

// clang-format on

TEST(SolidSyslogLwipRawDnsResolver, CreateSucceeds)
{
    CHECK(resolver != nullptr);
}

// clang-format off
TEST_GROUP(SolidSyslogLwipRawDnsResolverPool)
{
    struct SolidSyslogLwipRawDnsResolverConfig config = {};
    struct SolidSyslogResolver* pooled[SOLIDSYSLOG_LWIP_RAW_DNS_RESOLVER_POOL_SIZE] = {};
    struct SolidSyslogResolver* overflow                                            = nullptr;

    void setup() override
    {
        config.Sleep = FakeSleep;
    }

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogLwipRawDnsResolver_Destroy(handle);
            }
        }
        if (overflow != nullptr)
        {
            SolidSyslogLwipRawDnsResolver_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogLwipRawDnsResolver_Create(&config);
        }
    }
};

// clang-format on

TEST(SolidSyslogLwipRawDnsResolverPool, CreateWithNullConfigReturnsFallback)
{
    FillPool();

    overflow = SolidSyslogLwipRawDnsResolver_Create(nullptr);

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawDnsResolverPool, CreateWithNullSleepReturnsFallback)
{
    FillPool();
    struct SolidSyslogLwipRawDnsResolverConfig badConfig = {};
    badConfig.Sleep = nullptr;

    overflow = SolidSyslogLwipRawDnsResolver_Create(&badConfig);

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawDnsResolverPool, FillingPoolThenOverflowReturnsDistinctFallback)
{
    FillPool();

    overflow = SolidSyslogLwipRawDnsResolver_Create(&config);

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawDnsResolverPool, ExhaustedCreateReportsError)
{
    ErrorHandlerFake_Install(nullptr);
    FillPool();

    overflow = SolidSyslogLwipRawDnsResolver_Create(&config);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_ERROR, LwipRawDnsResolverErrorSource, LWIPRAWDNSRESOLVER_ERROR_POOL_EXHAUSTED);
}

TEST(SolidSyslogLwipRawDnsResolverPool, FallbackResolveReturnsFalse)
{
    FillPool();
    overflow = SolidSyslogLwipRawDnsResolver_Create(&config);
    struct SolidSyslogAddress* fallbackResult = SolidSyslogLwipRawAddress_Create();

    CHECK_FALSE(SolidSyslogResolver_Resolve(overflow, SOLIDSYSLOG_TRANSPORT_UDP, TEST_HOST, TEST_PORT, fallbackResult));

    SolidSyslogLwipRawAddress_Destroy(fallbackResult);
}

TEST(SolidSyslogLwipRawDnsResolverPool, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogLwipRawDnsResolver_Create(&config);

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawDnsResolverPool, CreateLocksOncePerSlotProbedWhenPoolIsFull)
{
    FillPool();
    ConfigLockFake_Install();

    overflow = SolidSyslogLwipRawDnsResolver_Create(&config);

    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_DNS_RESOLVER_POOL_SIZE, ConfigLockFake_LockCallCount());
    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_DNS_RESOLVER_POOL_SIZE, ConfigLockFake_UnlockCallCount());
}

TEST(SolidSyslogLwipRawDnsResolverPool, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogLwipRawDnsResolver_Create(&config);
    ConfigLockFake_Install();

    SolidSyslogLwipRawDnsResolver_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawDnsResolverPool, DestroyOfUnknownHandleDoesNotLock)
{
    ConfigLockFake_Install();
    struct SolidSyslogResolver stranger = {};

    SolidSyslogLwipRawDnsResolver_Destroy(&stranger);

    CALLED_FAKE(ConfigLockFake_Lock, NEVER);
    CALLED_FAKE(ConfigLockFake_Unlock, NEVER);
}

TEST(SolidSyslogLwipRawDnsResolverPool, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFake_Install(nullptr);
    struct SolidSyslogResolver stranger = {};

    SolidSyslogLwipRawDnsResolver_Destroy(&stranger);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawDnsResolverErrorSource, LWIPRAWDNSRESOLVER_ERROR_UNKNOWN_DESTROY);
}

TEST(SolidSyslogLwipRawDnsResolverPool, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogLwipRawDnsResolver_Create(&config);
    SolidSyslogLwipRawDnsResolver_Destroy(pooled[0]);
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogLwipRawDnsResolver_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawDnsResolverErrorSource, LWIPRAWDNSRESOLVER_ERROR_UNKNOWN_DESTROY);
}
