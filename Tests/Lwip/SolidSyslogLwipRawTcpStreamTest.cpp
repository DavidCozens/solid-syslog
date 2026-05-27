#include <stddef.h>
#include <stdint.h>

#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting;

#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "SolidSyslogLwipRawTcpStream.h"
#include "SolidSyslogLwipRawTcpStreamErrors.h"
#include "SolidSyslogNullStream.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogStreamDefinition.h"
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
#define CHECK_REPORTED(severity, source, code)                     \
    do                                                             \
    {                                                              \
        CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);                \
        LONGS_EQUAL((severity), ErrorHandlerFake_LastSeverity());  \
        POINTERS_EQUAL(&(source), ErrorHandlerFake_LastSource());  \
        UNSIGNED_LONGS_EQUAL((code), ErrorHandlerFake_LastCode()); \
    } while (0)

namespace
{
int FakeSleep_CallCount = 0;
int FakeSleep_LastMs = 0;

void FakeSleep_Reset()
{
    FakeSleep_CallCount = 0;
    FakeSleep_LastMs = 0;
}

extern "C" void FakeSleep(int milliseconds)
{
    FakeSleep_CallCount++;
    FakeSleep_LastMs = milliseconds;
}
} // namespace

// clang-format off
TEST_GROUP(SolidSyslogLwipRawTcpStream)
{
    struct SolidSyslogLwipRawTcpStreamConfig validConfig{};
    struct SolidSyslogStream* stream = nullptr;

    void setup() override
    {
        FakeSleep_Reset();
        validConfig = {};
        validConfig.Sleep = FakeSleep;
        stream = SolidSyslogLwipRawTcpStream_Create(&validConfig);
    }

    void teardown() override
    {
        if (stream != nullptr)
        {
            SolidSyslogLwipRawTcpStream_Destroy(stream);
        }
    }
};
// clang-format on

TEST(SolidSyslogLwipRawTcpStream, CreateReturnsNonNullStream)
{
    CHECK(stream != nullptr);
}

TEST(SolidSyslogLwipRawTcpStream, CreatedStreamIsNotTheNullStreamSingleton)
{
    CHECK(stream != SolidSyslogNullStream_Get());
}

TEST(SolidSyslogLwipRawTcpStream, DestroyReleasesSlotToPool)
{
    SolidSyslogLwipRawTcpStream_Destroy(stream);

    stream = SolidSyslogLwipRawTcpStream_Create(&validConfig);

    CHECK(stream != SolidSyslogNullStream_Get());
}

TEST(SolidSyslogLwipRawTcpStream, CreateWithNullConfigReturnsFallback)
{
    struct SolidSyslogStream* fallback = SolidSyslogLwipRawTcpStream_Create(nullptr);

    POINTERS_EQUAL(SolidSyslogNullStream_Get(), fallback);
}

TEST(SolidSyslogLwipRawTcpStream, CreateWithNullSleepReturnsFallback)
{
    struct SolidSyslogLwipRawTcpStreamConfig badConfig{};
    badConfig.Sleep = nullptr;

    struct SolidSyslogStream* fallback = SolidSyslogLwipRawTcpStream_Create(&badConfig);

    POINTERS_EQUAL(SolidSyslogNullStream_Get(), fallback);
}

// clang-format off
TEST_GROUP(SolidSyslogLwipRawTcpStreamPool)
{
    struct SolidSyslogLwipRawTcpStreamConfig validConfig{};
    struct SolidSyslogStream* pooled[SOLIDSYSLOG_LWIP_RAW_TCP_STREAM_POOL_SIZE] = {};
    struct SolidSyslogStream* overflow                                          = nullptr;

    void setup() override
    {
        FakeSleep_Reset();
        validConfig = {};
        validConfig.Sleep = FakeSleep;
    }

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogLwipRawTcpStream_Destroy(handle);
            }
        }
        if (overflow != nullptr)
        {
            SolidSyslogLwipRawTcpStream_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogLwipRawTcpStream_Create(&validConfig);
        }
    }
};
// clang-format on

TEST(SolidSyslogLwipRawTcpStreamPool, FillingPoolThenOverflowReturnsDistinctFallback)
{
    FillPool();

    overflow = SolidSyslogLwipRawTcpStream_Create(&validConfig);

    CHECK_IS_FALLBACK(overflow, pooled);
}

TEST(SolidSyslogLwipRawTcpStreamPool, ExhaustedCreateReportsError)
{
    ErrorHandlerFake_Install(nullptr);
    FillPool();

    overflow = SolidSyslogLwipRawTcpStream_Create(&validConfig);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_ERROR, LwipRawTcpStreamErrorSource, LWIPRAWTCPSTREAM_ERROR_POOL_EXHAUSTED);
}

TEST(SolidSyslogLwipRawTcpStreamPool, FallbackVtableMethodsAreNoOps)
{
    FillPool();
    overflow = SolidSyslogLwipRawTcpStream_Create(&validConfig);
    char buffer[1] = {0};

    /* NullStream's Open / Send return true so caller success paths are not
     * tripped; Read returns 0 (would-block) so callers don't tear the
     * connection down; Close is a no-op. */
    CHECK_TRUE(SolidSyslogStream_Open(overflow, nullptr));
    CHECK_TRUE(SolidSyslogStream_Send(overflow, "x", 1));
    LONGS_EQUAL(0, SolidSyslogStream_Read(overflow, buffer, sizeof(buffer)));
    SolidSyslogStream_Close(overflow);
}

TEST(SolidSyslogLwipRawTcpStreamPool, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogLwipRawTcpStream_Create(&validConfig);

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamPool, CreateLocksOncePerSlotProbedWhenPoolIsFull)
{
    FillPool();
    ConfigLockFake_Install();

    overflow = SolidSyslogLwipRawTcpStream_Create(&validConfig);

    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_TCP_STREAM_POOL_SIZE, ConfigLockFake_LockCallCount());
    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_TCP_STREAM_POOL_SIZE, ConfigLockFake_UnlockCallCount());
}

TEST(SolidSyslogLwipRawTcpStreamPool, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogLwipRawTcpStream_Create(&validConfig);
    ConfigLockFake_Install();

    SolidSyslogLwipRawTcpStream_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamPool, DestroyOfUnknownHandleDoesNotLock)
{
    ConfigLockFake_Install();
    struct SolidSyslogStream stranger = {};

    SolidSyslogLwipRawTcpStream_Destroy(&stranger);

    CALLED_FAKE(ConfigLockFake_Lock, NEVER);
    CALLED_FAKE(ConfigLockFake_Unlock, NEVER);
}

TEST(SolidSyslogLwipRawTcpStreamPool, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFake_Install(nullptr);
    struct SolidSyslogStream stranger = {};

    SolidSyslogLwipRawTcpStream_Destroy(&stranger);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawTcpStreamErrorSource, LWIPRAWTCPSTREAM_ERROR_UNKNOWN_DESTROY);
}

TEST(SolidSyslogLwipRawTcpStreamPool, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogLwipRawTcpStream_Create(&validConfig);
    struct SolidSyslogStream* stale = pooled[0];
    SolidSyslogLwipRawTcpStream_Destroy(pooled[0]);
    pooled[0] = nullptr;
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogLwipRawTcpStream_Destroy(stale);

    CHECK_REPORTED(SOLIDSYSLOG_SEVERITY_WARNING, LwipRawTcpStreamErrorSource, LWIPRAWTCPSTREAM_ERROR_UNKNOWN_DESTROY);
}
