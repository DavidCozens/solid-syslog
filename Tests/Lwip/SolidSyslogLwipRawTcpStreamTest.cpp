#include <stddef.h>
#include <stdint.h>

#include "TestUtils.h"
#include "CppUTest/TestHarness.h"

using namespace CososoTesting;

#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "LwipTcpFake.h"
#include "SolidSyslogLwipRawAddress.h"
#include "SolidSyslogLwipRawAddressPrivate.h"
#include "SolidSyslogLwipRawTcpStream.h"
#include "SolidSyslogLwipRawTcpStreamErrors.h"
#include "SolidSyslogNullStream.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogTunables.h"
#include "lwip/err.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"

static const uint16_t TEST_PORT = 514;

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
unsigned FakeSleep_CallCount = 0;
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

unsigned FakeGetConnectTimeoutMs_CallCount = 0;
void* FakeGetConnectTimeoutMs_LastContext = nullptr;
uint32_t FakeGetConnectTimeoutMs_ReturnValue = SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS;

void FakeGetConnectTimeoutMs_Reset()
{
    FakeGetConnectTimeoutMs_CallCount = 0;
    FakeGetConnectTimeoutMs_LastContext = reinterpret_cast<void*>(0x1U);
    FakeGetConnectTimeoutMs_ReturnValue = SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS;
}

extern "C" uint32_t FakeGetConnectTimeoutMs(void* context)
{
    FakeGetConnectTimeoutMs_CallCount++;
    FakeGetConnectTimeoutMs_LastContext = context;
    return FakeGetConnectTimeoutMs_ReturnValue;
}
} // namespace

/* Shared fixture: every TcpStream lifecycle test needs the fakes reset, a
 * fresh stream + address handle pair, teardown of both, and the leak
 * invariant — every tcp_pcb handed out by tcp_new must come back via
 * tcp_close / tcp_abort / null-via-tcp_err by the end of the test. */
// clang-format off
TEST_BASE(LwipRawTcpStreamTestBase)
{
    struct SolidSyslogLwipRawTcpStreamConfig config{};
    struct SolidSyslogStream* stream = nullptr;
    struct SolidSyslogAddress* address = nullptr;

    void createFakesAndHandles()
    {
        LwipTcpFake_Reset();
        FakeSleep_Reset();
        FakeGetConnectTimeoutMs_Reset();
        config = {};
        config.Sleep = FakeSleep;
        stream = SolidSyslogLwipRawTcpStream_Create(&config);
        address = SolidSyslogLwipRawAddress_Create();
        struct SolidSyslogLwipRawAddress* lwipAddress = SolidSyslogLwipRawAddress_As(address);
        IP4_ADDR(&lwipAddress->Ip, 127, 0, 0, 1);
        lwipAddress->Port = TEST_PORT;
    }

    void destroyHandlesAndCheckNoLeak() const
    {
        SolidSyslogLwipRawAddress_Destroy(address);
        SolidSyslogLwipRawTcpStream_Destroy(stream);
        LONGS_EQUAL_TEXT(0, LwipTcpFake_OutstandingPcbCount(), "leaked tcp_pcb past teardown");
    }
};

TEST_GROUP_BASE(SolidSyslogLwipRawTcpStream, LwipRawTcpStreamTestBase)
{
    void setup() override
    {
        createFakesAndHandles();
    }

    void teardown() override
    {
        destroyHandlesAndCheckNoLeak();
    }
};

TEST_GROUP_BASE(SolidSyslogLwipRawTcpStreamConnected, LwipRawTcpStreamTestBase)
{
    void setup() override
    {
        createFakesAndHandles();
        SolidSyslogStream_Open(stream, address);
    }

    void teardown() override
    {
        destroyHandlesAndCheckNoLeak();
    }
};
// clang-format on

/* ------------------------------------------------------------------
 * Created-but-not-opened tests
 * ----------------------------------------------------------------*/

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

    stream = SolidSyslogLwipRawTcpStream_Create(&config);

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

TEST(SolidSyslogLwipRawTcpStream, CloseBeforeOpenIsNoOp)
{
    SolidSyslogStream_Close(stream);

    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
    CALLED_FAKE(LwipTcpFake_TcpAbort, NEVER);
}

TEST(SolidSyslogLwipRawTcpStream, OpenCallsTcpNew)
{
    SolidSyslogStream_Open(stream, address);

    CALLED_FAKE(LwipTcpFake_TcpNew, ONCE);
}

TEST(SolidSyslogLwipRawTcpStream, OpenReturnsTrueOnSuccessfulConnect)
{
    CHECK_TRUE(SolidSyslogStream_Open(stream, address));
}

TEST(SolidSyslogLwipRawTcpStream, OpenReturnsFalseWhenTcpNewFails)
{
    LwipTcpFake_SetTcpNewFails(true);

    CHECK_FALSE(SolidSyslogStream_Open(stream, address));
    CALLED_FAKE(LwipTcpFake_TcpConnect, NEVER);
}

TEST(SolidSyslogLwipRawTcpStream, OpenSetsKeepaliveOnPcb)
{
    SolidSyslogStream_Open(stream, address);

    CHECK((LwipTcpFake_LastTcpNewReturned()->so_options & SOF_KEEPALIVE) != 0);
}

TEST(SolidSyslogLwipRawTcpStream, OpenRegistersTcpArgRecvErrSentCallbacks)
{
    SolidSyslogStream_Open(stream, address);

    CALLED_FAKE(LwipTcpFake_TcpArg, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpRecv, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpErr, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpSent, ONCE);
    CHECK(LwipTcpFake_LastRecvFn() != nullptr);
    CHECK(LwipTcpFake_LastErrFn() != nullptr);
    CHECK(LwipTcpFake_LastSentFn() != nullptr);
    CHECK(LwipTcpFake_LastCallbackArg() != nullptr);
}

TEST(SolidSyslogLwipRawTcpStream, OpenCallsTcpConnectWithAddressIpAndPort)
{
    SolidSyslogStream_Open(stream, address);

    CALLED_FAKE(LwipTcpFake_TcpConnect, ONCE);
    POINTERS_EQUAL(LwipTcpFake_LastTcpNewReturned(), LwipTcpFake_LastConnectPcb());
    POINTERS_EQUAL(&SolidSyslogLwipRawAddress_AsConst(address)->Ip, LwipTcpFake_LastConnectIpaddr());
    LONGS_EQUAL(TEST_PORT, LwipTcpFake_LastConnectPort());
    CHECK(LwipTcpFake_LastConnectedFn() != nullptr);
}

TEST(SolidSyslogLwipRawTcpStream, OpenReturnsFalseAndAbortsWhenConnectedCallbackFiresErrored)
{
    LwipTcpFake_SetConnectCallbackResult(ERR_RST);

    CHECK_FALSE(SolidSyslogStream_Open(stream, address));
    CALLED_FAKE(LwipTcpFake_TcpAbort, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
}

TEST(SolidSyslogLwipRawTcpStream, OpenReturnsFalseAndAbortsOnImmediateTcpConnectError)
{
    LwipTcpFake_SetTcpConnectError(ERR_VAL);

    CHECK_FALSE(SolidSyslogStream_Open(stream, address));
    CALLED_FAKE(LwipTcpFake_TcpAbort, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
}

TEST(SolidSyslogLwipRawTcpStream, OpenReturnsFalseAndAbortsOnConnectTimeout)
{
    LwipTcpFake_SetConnectCallbackFires(false);

    CHECK_FALSE(SolidSyslogStream_Open(stream, address));
    CALLED_FAKE(LwipTcpFake_TcpAbort, ONCE);
    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
}

TEST(SolidSyslogLwipRawTcpStream, OpenSleepsBetweenPollsDuringTimeoutPath)
{
    LwipTcpFake_SetConnectCallbackFires(false);

    SolidSyslogStream_Open(stream, address);

    /* timeout / poll periods → exactly that many sleeps before giving up. */
    LONGS_EQUAL(SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS / SOLIDSYSLOG_LWIP_RAW_TCP_CONNECT_POLL_MS, FakeSleep_CallCount);
    LONGS_EQUAL(SOLIDSYSLOG_LWIP_RAW_TCP_CONNECT_POLL_MS, FakeSleep_LastMs);
}

TEST(SolidSyslogLwipRawTcpStream, OpenHappyPathDoesNotSleep)
{
    SolidSyslogStream_Open(stream, address);

    LONGS_EQUAL(0, FakeSleep_CallCount);
}

TEST(SolidSyslogLwipRawTcpStream, OpenRespectsRuntimeTunableConnectTimeout)
{
    LwipTcpFake_SetConnectCallbackFires(false);
    /* Re-create the stream with a getter installed. The default fixture's
     * stream uses NULL getter (falls back to the compile-time tunable);
     * we want to verify the getter is honoured on the timeout deadline. */
    SolidSyslogLwipRawTcpStream_Destroy(stream);
    config.GetConnectTimeoutMs = FakeGetConnectTimeoutMs;
    config.ConnectTimeoutContext = reinterpret_cast<void*>(0xABCDU);
    FakeGetConnectTimeoutMs_ReturnValue = 20U;
    stream = SolidSyslogLwipRawTcpStream_Create(&config);

    SolidSyslogStream_Open(stream, address);

    /* 20 ms / 10 ms poll = 2 polls. */
    LONGS_EQUAL(2, FakeSleep_CallCount);
    CHECK(FakeGetConnectTimeoutMs_CallCount >= 1U);
    POINTERS_EQUAL(reinterpret_cast<void*>(0xABCDU), FakeGetConnectTimeoutMs_LastContext);
}

/* ------------------------------------------------------------------
 * Open-already-called tests
 * ----------------------------------------------------------------*/

TEST(SolidSyslogLwipRawTcpStreamConnected, ReopenDoesNotAllocateNewPcb)
{
    CHECK_TRUE(SolidSyslogStream_Open(stream, address));

    CALLED_FAKE(LwipTcpFake_TcpNew, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, CloseCallsTcpCloseOnOpenPcb)
{
    SolidSyslogStream_Close(stream);

    CALLED_FAKE(LwipTcpFake_TcpClose, ONCE);
    POINTERS_EQUAL(LwipTcpFake_LastTcpNewReturned(), LwipTcpFake_LastClosePcb());
}

TEST(SolidSyslogLwipRawTcpStreamConnected, SecondCloseDoesNotCloseAgain)
{
    SolidSyslogStream_Close(stream);

    SolidSyslogStream_Close(stream);

    CALLED_FAKE(LwipTcpFake_TcpClose, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, CloseThenOpenAllocatesFreshPcb)
{
    SolidSyslogStream_Close(stream);

    SolidSyslogStream_Open(stream, address);

    CALLED_FAKE(LwipTcpFake_TcpNew, TWICE);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, DestroyClosesOpenPcb)
{
    SolidSyslogLwipRawTcpStream_Destroy(stream);
    stream = nullptr;

    CALLED_FAKE(LwipTcpFake_TcpClose, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, DestroyAfterCloseDoesNotCloseAgain)
{
    SolidSyslogStream_Close(stream);

    SolidSyslogLwipRawTcpStream_Destroy(stream);
    stream = nullptr;

    CALLED_FAKE(LwipTcpFake_TcpClose, ONCE);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, TcpErrCallbackReleasesPcbWithoutCallingTcpClose)
{
    /* Drive the err callback the wrapper registered. lwIP releases the
     * pcb upstream before invoking err — the wrapper must null its Pcb
     * field and NOT call tcp_close (use-after-free). */
    tcp_err_fn errCb = LwipTcpFake_LastErrFn();
    void* arg = LwipTcpFake_LastCallbackArg();
    errCb(arg, ERR_RST);
    LwipTcpFake_NotePcbReleasedByErr();

    SolidSyslogStream_Close(stream);

    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
}

TEST(SolidSyslogLwipRawTcpStreamConnected, RecvCallbackReturnsErrOkAsNoOpStub)
{
    /* Real RX queue handling lands in the Send/Read slice — this slot's
     * stub returns ERR_OK so lwIP sees the bytes as accepted. */
    tcp_recv_fn recvCb = LwipTcpFake_LastRecvFn();

    LONGS_EQUAL(
        ERR_OK,
        recvCb(LwipTcpFake_LastCallbackArg(), LwipTcpFake_LastTcpNewReturned(), nullptr, ERR_OK)
    );
}

TEST(SolidSyslogLwipRawTcpStreamConnected, SentCallbackReturnsErrOkAsNoOpStub)
{
    /* TCP_WRITE_FLAG_COPY means caller buffers are released at Send return —
     * no per-ACK accounting needed. The slot exists because lwIP wants the
     * callback set when the pcb is wired. */
    tcp_sent_fn sentCb = LwipTcpFake_LastSentFn();

    LONGS_EQUAL(ERR_OK, sentCb(LwipTcpFake_LastCallbackArg(), LwipTcpFake_LastTcpNewReturned(), 0));
}

TEST(SolidSyslogLwipRawTcpStreamConnected, DestroyAfterTcpErrDoesNotCallTcpClose)
{
    tcp_err_fn errCb = LwipTcpFake_LastErrFn();
    void* arg = LwipTcpFake_LastCallbackArg();
    errCb(arg, ERR_RST);
    LwipTcpFake_NotePcbReleasedByErr();

    SolidSyslogLwipRawTcpStream_Destroy(stream);
    stream = nullptr;

    CALLED_FAKE(LwipTcpFake_TcpClose, NEVER);
}

/* ------------------------------------------------------------------
 * Pool tests — handed-out handles never call lwIP, so they don't need
 * the fake state. Same TEST_GROUP shape as Commit 1.
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogLwipRawTcpStreamPool)
{
    struct SolidSyslogLwipRawTcpStreamConfig validConfig{};
    struct SolidSyslogStream* pooled[SOLIDSYSLOG_LWIP_RAW_TCP_STREAM_POOL_SIZE] = {};
    struct SolidSyslogStream* overflow                                          = nullptr;

    void setup() override
    {
        LwipTcpFake_Reset();
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
