#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting;

#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "LwipUdpFake.h"
#include "SolidSyslogDatagram.h"
#include "SolidSyslogDatagramDefinition.h"
#include "SolidSyslogLwipRawAddress.h"
#include "SolidSyslogLwipRawDatagram.h"
#include "SolidSyslogLwipRawDatagramErrors.h"
#include "SolidSyslogNullDatagram.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTunables.h"

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
// Use after the act-phase of a test that expects exactly one SolidSyslog_Error call.
#define CHECK_REPORTED(severity, source, code)                     \
    do                                                             \
    {                                                              \
        CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);                \
        LONGS_EQUAL((severity), ErrorHandlerFake_LastSeverity());  \
        POINTERS_EQUAL(&(source), ErrorHandlerFake_LastSource());  \
        UNSIGNED_LONGS_EQUAL((code), ErrorHandlerFake_LastCode()); \
    } while (0)

// clang-format off
TEST_GROUP(SolidSyslogLwipRawDatagram)
{
    struct SolidSyslogDatagram* datagram = nullptr;
    struct SolidSyslogAddress* address = nullptr;

    void setup() override
    {
        LwipUdpFake_Reset();
        datagram = SolidSyslogLwipRawDatagram_Create();
        address = SolidSyslogLwipRawAddress_Create();
    }

    void teardown() override
    {
        SolidSyslogLwipRawAddress_Destroy(address);
        SolidSyslogLwipRawDatagram_Destroy(datagram);
    }
};
// clang-format on

TEST(SolidSyslogLwipRawDatagram, CreateReturnsNonNullDatagram)
{
    CHECK(datagram != nullptr);
}

TEST(SolidSyslogLwipRawDatagram, DestroyReleasesSlotToPool)
{
    SolidSyslogLwipRawDatagram_Destroy(datagram);

    datagram = SolidSyslogLwipRawDatagram_Create();

    CHECK(datagram != SolidSyslogNullDatagram_Get());
}

TEST(SolidSyslogLwipRawDatagram, OpenSucceeds)
{
    CHECK_TRUE(SolidSyslogDatagram_Open(datagram));
}

TEST(SolidSyslogLwipRawDatagram, CloseSucceeds)
{
    SolidSyslogDatagram_Close(datagram);
}

TEST(SolidSyslogLwipRawDatagram, OpenCallsUdpNew)
{
    SolidSyslogDatagram_Open(datagram);

    CALLED_FAKE(LwipUdpFake_UdpNew, ONCE);
}

TEST(SolidSyslogLwipRawDatagram, CloseCallsUdpRemoveOnOpenPcb)
{
    SolidSyslogDatagram_Open(datagram);

    SolidSyslogDatagram_Close(datagram);

    CALLED_FAKE(LwipUdpFake_UdpRemove, ONCE);
    POINTERS_EQUAL(LwipUdpFake_LastUdpRemovePcb(), LwipUdpFake_LastUdpNewReturned());
}

TEST(SolidSyslogLwipRawDatagram, OpenIsIdempotent)
{
    SolidSyslogDatagram_Open(datagram);

    CHECK_TRUE(SolidSyslogDatagram_Open(datagram));
    CALLED_FAKE(LwipUdpFake_UdpNew, ONCE);
}

TEST(SolidSyslogLwipRawDatagram, CloseIsIdempotent)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_Close(datagram);

    SolidSyslogDatagram_Close(datagram);

    CALLED_FAKE(LwipUdpFake_UdpRemove, ONCE);
}

TEST(SolidSyslogLwipRawDatagram, CloseWithoutOpenIsNoOp)
{
    SolidSyslogDatagram_Close(datagram);

    CALLED_FAKE(LwipUdpFake_UdpRemove, NEVER);
}

TEST(SolidSyslogLwipRawDatagram, OpenReturnsFalseWhenUdpNewFails)
{
    LwipUdpFake_SetUdpNewFails(true);

    CHECK_FALSE(SolidSyslogDatagram_Open(datagram));
}

TEST(SolidSyslogLwipRawDatagram, CloseAndReopenCreatesFreshPcb)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_Close(datagram);

    SolidSyslogDatagram_Open(datagram);

    CALLED_FAKE(LwipUdpFake_UdpNew, TWICE);
}

TEST(SolidSyslogLwipRawDatagram, DestroyClosesOpenPcb)
{
    SolidSyslogDatagram_Open(datagram);

    SolidSyslogLwipRawDatagram_Destroy(datagram);
    datagram = nullptr;

    CALLED_FAKE(LwipUdpFake_UdpRemove, ONCE);
}

TEST(SolidSyslogLwipRawDatagram, DestroyAfterCloseDoesNotRemoveAgain)
{
    SolidSyslogDatagram_Open(datagram);
    SolidSyslogDatagram_Close(datagram);

    SolidSyslogLwipRawDatagram_Destroy(datagram);
    datagram = nullptr;

    CALLED_FAKE(LwipUdpFake_UdpRemove, ONCE);
}

// clang-format off
TEST_GROUP(SolidSyslogLwipRawDatagramPool)
{
    struct SolidSyslogDatagram* pooled[SOLIDSYSLOG_LWIP_RAW_DATAGRAM_POOL_SIZE] = {};
    struct SolidSyslogDatagram* overflow                                         = nullptr;

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogLwipRawDatagram_Destroy(handle);
            }
        }
        if (overflow != nullptr)
        {
            SolidSyslogLwipRawDatagram_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogLwipRawDatagram_Create();
        }
    }
};
// clang-format on

TEST(SolidSyslogLwipRawDatagramPool, FillingPoolThenOverflowReturnsDistinctFallback)
{
    FillPool();

    overflow = SolidSyslogLwipRawDatagram_Create();

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawDatagramPool, ExhaustedCreateReportsError)
{
    ErrorHandlerFake_Install(nullptr);
    FillPool();

    overflow = SolidSyslogLwipRawDatagram_Create();

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_ERROR, LwipRawDatagramErrorSource, LWIPRAWDATAGRAM_ERROR_POOL_EXHAUSTED);
}

TEST(SolidSyslogLwipRawDatagramPool, FallbackVtableMethodsAreNoOps)
{
    FillPool();
    overflow = SolidSyslogLwipRawDatagram_Create();
    struct SolidSyslogAddress* localAddr = SolidSyslogLwipRawAddress_Create();

    /* NullDatagram's Open returns true so caller success paths are not
     * tripped; SendTo returns SENT; no underlying lwIP API is invoked
     * because the production-side vtable is never wired on the fallback
     * handle. */
    CHECK_TRUE(SolidSyslogDatagram_Open(overflow));
    LONGS_EQUAL(
        SOLIDSYSLOG_DATAGRAM_SEND_RESULT_SENT,
        SolidSyslogDatagram_SendTo(overflow, "x", 1, localAddr)
    );
    SolidSyslogDatagram_Close(overflow);

    SolidSyslogLwipRawAddress_Destroy(localAddr);
}

TEST(SolidSyslogLwipRawDatagramPool, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogLwipRawDatagram_Create();

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawDatagramPool, CreateLocksOncePerSlotProbedWhenPoolIsFull)
{
    FillPool();
    ConfigLockFake_Install();

    overflow = SolidSyslogLwipRawDatagram_Create();

    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_DATAGRAM_POOL_SIZE, ConfigLockFake_LockCallCount());
    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_DATAGRAM_POOL_SIZE, ConfigLockFake_UnlockCallCount());
}

TEST(SolidSyslogLwipRawDatagramPool, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogLwipRawDatagram_Create();
    ConfigLockFake_Install();

    SolidSyslogLwipRawDatagram_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawDatagramPool, DestroyOfUnknownHandleDoesNotLock)
{
    ConfigLockFake_Install();
    struct SolidSyslogDatagram stranger = {};

    SolidSyslogLwipRawDatagram_Destroy(&stranger);

    CALLED_FAKE(ConfigLockFake_Lock, NEVER);
    CALLED_FAKE(ConfigLockFake_Unlock, NEVER);
}

TEST(SolidSyslogLwipRawDatagramPool, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFake_Install(nullptr);
    struct SolidSyslogDatagram stranger = {};

    SolidSyslogLwipRawDatagram_Destroy(&stranger);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawDatagramErrorSource, LWIPRAWDATAGRAM_ERROR_UNKNOWN_DESTROY);
}

TEST(SolidSyslogLwipRawDatagramPool, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogLwipRawDatagram_Create();
    SolidSyslogLwipRawDatagram_Destroy(pooled[0]);
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogLwipRawDatagram_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawDatagramErrorSource, LWIPRAWDATAGRAM_ERROR_UNKNOWN_DESTROY);
}
