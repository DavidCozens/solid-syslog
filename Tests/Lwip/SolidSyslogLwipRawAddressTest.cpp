#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting; // NOLINT(google-build-using-namespace) -- test-file scope only; brings NEVER/ONCE/TWICE/THRICE into scope for the CALLED_*
    // macros

#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "SolidSyslogLwipRawAddress.h"
#include "SolidSyslogLwipRawAddressErrors.h"
#include "SolidSyslogLwipRawAddressPrivate.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTunables.h"

#include "lwip/ip_addr.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while) -- macros preserve __FILE__/__LINE__ at the call site

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

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while)

// clang-format off
TEST_GROUP(SolidSyslogLwipRawAddress)
{
    struct SolidSyslogAddress* address = nullptr;

    void setup() override
    {
        address = SolidSyslogLwipRawAddress_Create();
    }

    void teardown() override
    {
        SolidSyslogLwipRawAddress_Destroy(address);
    }
};

// clang-format on

TEST(SolidSyslogLwipRawAddress, CreateReturnsNonNull)
{
    CHECK(address != nullptr);
}

TEST(SolidSyslogLwipRawAddress, IpFieldRoundTripsBytes)
{
    struct SolidSyslogLwipRawAddress* self = SolidSyslogLwipRawAddress_As(address);
    IP4_ADDR(&self->Ip, 127, 0, 0, 1);

    ip_addr_t expected;
    IP4_ADDR(&expected, 127, 0, 0, 1);
    LONGS_EQUAL(
        ip4_addr_get_u32(ip_2_ip4(&expected)),
        ip4_addr_get_u32(ip_2_ip4(&SolidSyslogLwipRawAddress_AsConst(address)->Ip))
    );
}

TEST(SolidSyslogLwipRawAddress, PortFieldRoundTripsValue)
{
    SolidSyslogLwipRawAddress_As(address)->Port = 514;

    LONGS_EQUAL(514, SolidSyslogLwipRawAddress_AsConst(address)->Port);
}

TEST(SolidSyslogLwipRawAddress, CreateZeroesIpAndPortFromAnyPriorSlotContents)
{
    struct SolidSyslogLwipRawAddress* self = SolidSyslogLwipRawAddress_As(address);
    IP4_ADDR(&self->Ip, 1, 2, 3, 4);
    self->Port = 9999;
    SolidSyslogLwipRawAddress_Destroy(address);

    address = SolidSyslogLwipRawAddress_Create();

    const struct SolidSyslogLwipRawAddress* fresh = SolidSyslogLwipRawAddress_AsConst(address);
    LONGS_EQUAL(0, ip4_addr_get_u32(ip_2_ip4(&fresh->Ip)));
    LONGS_EQUAL(0, fresh->Port);
}

// clang-format off
TEST_GROUP(SolidSyslogLwipRawAddressPool)
{
    // cppcheck-suppress constVariable -- assigned in test bodies; cppcheck does not model CppUTest lifecycle
    struct SolidSyslogAddress* pooled[SOLIDSYSLOG_ADDRESS_POOL_SIZE] = {};
    struct SolidSyslogAddress* overflow                              = nullptr;

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogLwipRawAddress_Destroy(handle);
            }
        }
        // cppcheck-suppress knownConditionTrueFalse -- assigned in test bodies; cppcheck does not model CppUTest lifecycle
        if (overflow != nullptr)
        {
            SolidSyslogLwipRawAddress_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogLwipRawAddress_Create();
        }
    }
};

// clang-format on

TEST(SolidSyslogLwipRawAddressPool, FillingPoolThenOverflowReturnsDistinctFallback)
{
    FillPool();

    overflow = SolidSyslogLwipRawAddress_Create();

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawAddressPool, ExhaustedCreateReportsError)
{
    ErrorHandlerFake_Install(nullptr);
    FillPool();

    overflow = SolidSyslogLwipRawAddress_Create();

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_ERROR, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&LwipRawAddressErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(LWIPRAWADDRESS_ERROR_POOL_EXHAUSTED, ErrorHandlerFake_LastCode());
}

TEST(SolidSyslogLwipRawAddressPool, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogLwipRawAddress_Create();

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawAddressPool, CreateLocksOncePerSlotProbedWhenPoolIsFull)
{
    FillPool();
    ConfigLockFake_Install();

    overflow = SolidSyslogLwipRawAddress_Create();

    LONGS_EQUAL(SOLIDSYSLOG_ADDRESS_POOL_SIZE, ConfigLockFake_LockCallCount());
    LONGS_EQUAL(SOLIDSYSLOG_ADDRESS_POOL_SIZE, ConfigLockFake_UnlockCallCount());
}

TEST(SolidSyslogLwipRawAddressPool, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogLwipRawAddress_Create();
    ConfigLockFake_Install();

    SolidSyslogLwipRawAddress_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawAddressPool, DestroyOfUnknownHandleDoesNotLock)
{
    ConfigLockFake_Install();
    char stranger = 0;

    SolidSyslogLwipRawAddress_Destroy((struct SolidSyslogAddress*) &stranger);

    CALLED_FAKE(ConfigLockFake_Lock, NEVER);
    CALLED_FAKE(ConfigLockFake_Unlock, NEVER);
}

TEST(SolidSyslogLwipRawAddressPool, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFake_Install(nullptr);
    char stranger = 0;

    SolidSyslogLwipRawAddress_Destroy((struct SolidSyslogAddress*) &stranger);

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_WARNING, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&LwipRawAddressErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(LWIPRAWADDRESS_ERROR_UNKNOWN_DESTROY, ErrorHandlerFake_LastCode());
}

TEST(SolidSyslogLwipRawAddressPool, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogLwipRawAddress_Create();
    SolidSyslogLwipRawAddress_Destroy(pooled[0]);
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogLwipRawAddress_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_WARNING, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&LwipRawAddressErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(LWIPRAWADDRESS_ERROR_UNKNOWN_DESTROY, ErrorHandlerFake_LastCode());
}
