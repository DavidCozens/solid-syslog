#include "CppUTest/TestHarness.h"
#include "PosixUdpSender.h"

// clang-format off
TEST_GROUP(PosixUdpSender)
{
    struct PosixUdpSender_Config config = {0};
    struct SolidSyslog_Sender*   sender = nullptr;

    void setup() override
    {
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

// clang-format off
// Test list — S2.1: Walking Skeleton — PosixUdpSender transmits a buffer
//
// Z — Zero
//   [x] CreateDestroyWorksWithoutCrashing
//
// O — One
//   [ ] A single Send call results in exactly one call to SocketSpy sendto
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
