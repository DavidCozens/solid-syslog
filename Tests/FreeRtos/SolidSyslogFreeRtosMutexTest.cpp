#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting; // NOLINT(google-build-using-namespace) -- test-file scope only; brings NEVER/ONCE/TWICE/THRICE into scope for the CALLED_*
    // macros

#include "ConfigLockFake.h"
#include "ErrorHandlerFakeEx.h"
#include "FreeRtosSemaphoreFake.h"
#include "SolidSyslogFreeRtosMutex.h"
#include "SolidSyslogFreeRtosMutexErrors.h"
#include "SolidSyslogMutex.h"
#include "SolidSyslogMutexDefinition.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTunables.h"

#include "FreeRTOS.h"
#include "semphr.h"

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
TEST_GROUP(SolidSyslogFreeRtosMutex)
{
    struct SolidSyslogMutex* mutex = nullptr;

    void setup() override
    {
        FreeRtosSemaphoreFake_Reset();
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        mutex = SolidSyslogFreeRtosMutex_Create();
    }

    void teardown() override
    {
        SolidSyslogFreeRtosMutex_Destroy(mutex);
    }
};

// clang-format on

TEST(SolidSyslogFreeRtosMutex, CreateCallsCreateMutexStaticOnce)
{
    CALLED_FAKE(FreeRtosSemaphoreFake_CreateMutexStatic, ONCE);
}

TEST(SolidSyslogFreeRtosMutex, LockCallsSemaphoreTakeOnce)
{
    SolidSyslogMutex_Lock(mutex);

    CALLED_FAKE(FreeRtosSemaphoreFake_SemaphoreTake, ONCE);
}

TEST(SolidSyslogFreeRtosMutex, UnlockCallsSemaphoreGiveOnce)
{
    SolidSyslogMutex_Unlock(mutex);

    CALLED_FAKE(FreeRtosSemaphoreFake_SemaphoreGive, ONCE);
}

TEST(SolidSyslogFreeRtosMutex, DestroyCallsSemaphoreDeleteOnce)
{
    SolidSyslogFreeRtosMutex_Destroy(mutex);
    mutex = nullptr;

    CALLED_FAKE(FreeRtosSemaphoreFake_SemaphoreDelete, ONCE);
}

// clang-format off
TEST_GROUP(SolidSyslogFreeRtosMutexPool)
{
    // cppcheck-suppress constVariable -- assigned in test bodies; cppcheck does not model CppUTest lifecycle
    struct SolidSyslogMutex* pooled[SOLIDSYSLOG_FREE_RTOS_MUTEX_POOL_SIZE] = {};
    struct SolidSyslogMutex* overflow                                      = nullptr;

    void setup() override
    {
        FreeRtosSemaphoreFake_Reset();
    }

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogFreeRtosMutex_Destroy(handle);
            }
        }
        // cppcheck-suppress knownConditionTrueFalse -- assigned in test bodies; cppcheck does not model CppUTest lifecycle
        if (overflow != nullptr)
        {
            SolidSyslogFreeRtosMutex_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogFreeRtosMutex_Create();
        }
    }
};

// clang-format on

TEST(SolidSyslogFreeRtosMutexPool, FillingPoolThenOverflowReturnsDistinctFallback)
{
    FillPool();

    overflow = SolidSyslogFreeRtosMutex_Create();

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogFreeRtosMutexPool, ExhaustedCreateReportsError)
{
    ErrorHandlerFakeEx_Install(nullptr);
    FillPool();

    overflow = SolidSyslogFreeRtosMutex_Create();

    CALLED_FAKE(ErrorHandlerFakeEx_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_ERROR, ErrorHandlerFakeEx_LastSeverity());
    POINTERS_EQUAL(&FreeRtosMutexErrorSource, ErrorHandlerFakeEx_LastSource());
    UNSIGNED_LONGS_EQUAL(FREERTOSMUTEX_ERROR_POOL_EXHAUSTED, ErrorHandlerFakeEx_LastCode());
}

TEST(SolidSyslogFreeRtosMutexPool, FallbackLockUnlockAreNoOps)
{
    FillPool();
    FreeRtosSemaphoreFake_Reset();
    overflow = SolidSyslogFreeRtosMutex_Create();

    SolidSyslogMutex_Lock(overflow);
    SolidSyslogMutex_Unlock(overflow);

    CALLED_FAKE(FreeRtosSemaphoreFake_SemaphoreTake, NEVER);
    CALLED_FAKE(FreeRtosSemaphoreFake_SemaphoreGive, NEVER);
}

TEST(SolidSyslogFreeRtosMutexPool, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogFreeRtosMutex_Create();

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogFreeRtosMutexPool, CreateLocksOncePerSlotProbedWhenPoolIsFull)
{
    FillPool();
    ConfigLockFake_Install();

    overflow = SolidSyslogFreeRtosMutex_Create();

    LONGS_EQUAL(SOLIDSYSLOG_FREE_RTOS_MUTEX_POOL_SIZE, ConfigLockFake_LockCallCount());
    LONGS_EQUAL(SOLIDSYSLOG_FREE_RTOS_MUTEX_POOL_SIZE, ConfigLockFake_UnlockCallCount());
}

TEST(SolidSyslogFreeRtosMutexPool, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogFreeRtosMutex_Create();
    ConfigLockFake_Install();

    SolidSyslogFreeRtosMutex_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogFreeRtosMutexPool, DestroyOfUnknownHandleDoesNotLock)
{
    ConfigLockFake_Install();
    struct SolidSyslogMutex stranger = {};

    SolidSyslogFreeRtosMutex_Destroy(&stranger);

    CALLED_FAKE(ConfigLockFake_Lock, NEVER);
    CALLED_FAKE(ConfigLockFake_Unlock, NEVER);
}

TEST(SolidSyslogFreeRtosMutexPool, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFakeEx_Install(nullptr);
    struct SolidSyslogMutex stranger = {};

    SolidSyslogFreeRtosMutex_Destroy(&stranger);

    CALLED_FAKE(ErrorHandlerFakeEx_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_WARNING, ErrorHandlerFakeEx_LastSeverity());
    POINTERS_EQUAL(&FreeRtosMutexErrorSource, ErrorHandlerFakeEx_LastSource());
    UNSIGNED_LONGS_EQUAL(FREERTOSMUTEX_ERROR_UNKNOWN_DESTROY, ErrorHandlerFakeEx_LastCode());
}

TEST(SolidSyslogFreeRtosMutexPool, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogFreeRtosMutex_Create();
    SolidSyslogFreeRtosMutex_Destroy(pooled[0]);
    ErrorHandlerFakeEx_Install(nullptr);

    SolidSyslogFreeRtosMutex_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ErrorHandlerFakeEx_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_WARNING, ErrorHandlerFakeEx_LastSeverity());
    POINTERS_EQUAL(&FreeRtosMutexErrorSource, ErrorHandlerFakeEx_LastSource());
    UNSIGNED_LONGS_EQUAL(FREERTOSMUTEX_ERROR_UNKNOWN_DESTROY, ErrorHandlerFakeEx_LastCode());
}
