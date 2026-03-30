#include "CppUTest/TestHarness.h"
#include "PosixUdpSender.h"
#include "SocketSpy.h"

// clang-format off
TEST_GROUP(PosixUdpSender)
{
    struct PosixUdpSender_Config config = {0};
    // cppcheck-suppress constVariablePointer -- Send requires non-const self; false positive from macro expansion
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslog_Sender* sender = nullptr;

    void setup() override
    {
        SocketSpy_Reset();
        // cppcheck-suppress unreadVariable -- read by teardown and tests; cppcheck does not model CppUTest lifecycle
        sender = PosixUdpSender_Create(&config);
    }

    void teardown() override
    {
        PosixUdpSender_Destroy(sender);
    }
};
// clang-format on

TEST(PosixUdpSender, CreateDestroyWorksWithoutCrashing)
{
}

TEST(PosixUdpSender, SingleSendResultsInOneSendtoCall)
{
    sender->Send(sender, "hello", 5);
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

// clang-format off
// Test list — S2.1: Walking Skeleton — PosixUdpSender transmits a buffer
//
// Z — Zero
//   [x] CreateDestroyWorksWithoutCrashing
//
// O — One
//   [x] A single Send call results in exactly one call to SocketSpy sendto
//   [ ] SocketSpy sendto called with correct buffer contents
//   [ ] SocketSpy sendto called with correct destination port
//
// B — Boundaries
//   [ ] Maximum RFC 5426 recommended message size (1024 bytes) transmitted without truncation
//
// E — Exceptions (deferred — error handling phase)
//   [ ] Create with NULL config returns NULL
//   [ ] Create with valid config returns non-NULL sender
//   [ ] Send called with NULL buffer does nothing, does not crash
//   [ ] Send called with zero length does nothing, does not crash
//   [ ] socket returning -1 handled gracefully — Create returns NULL or Send is a no-op
//   [ ] Unreachable host does not crash
//
// S — Simple scenario
//   [ ] socket called once with AF_INET and SOCK_DGRAM, sendto called once with correct
//       address family and port, close called once on Destroy
// clang-format on
