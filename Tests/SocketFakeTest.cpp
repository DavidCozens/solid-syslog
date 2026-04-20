#include "CppUTest/TestHarness.h"
#include "SocketFake.h"

#include <sys/socket.h>

// clang-format off
TEST_GROUP(SocketFake)
{
    void setup() override { SocketFake_Reset(); }
};
// clang-format on

TEST(SocketFake, RecvIncrementsCount)
{
    char buf[16];
    recv(3, buf, sizeof(buf), 0);
    LONGS_EQUAL(1, SocketFake_RecvCallCount());
}
