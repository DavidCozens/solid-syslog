#include "CppUTest/TestHarness.h"
#include "SolidSyslogAtomicOpsDefinition.h"
#include "SolidSyslogWindowsAtomicOps.h"

// clang-format off
TEST_GROUP(SolidSyslogWindowsAtomicOps)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogAtomicOps* ops;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        ops = SolidSyslogWindowsAtomicOps_Create();
    }

    void teardown() override
    {
        SolidSyslogWindowsAtomicOps_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogWindowsAtomicOps, CreateReturnsNonNull)
{
    CHECK(ops != nullptr);
}

TEST(SolidSyslogWindowsAtomicOps, LoadAfterCreateReturns0)
{
    LONGS_EQUAL(0, ops->Load(ops));
}

TEST(SolidSyslogWindowsAtomicOps, LoadReturnsValueCommittedByCompareAndSwap)
{
    CHECK_TRUE(ops->CompareAndSwap(ops, 0, 42));
    LONGS_EQUAL(42, ops->Load(ops));
}

TEST(SolidSyslogWindowsAtomicOps, CompareAndSwapWithMismatchedExpectedReturnsFalseAndLeavesValueUnchanged)
{
    CHECK_FALSE(ops->CompareAndSwap(ops, 99, 42));
    LONGS_EQUAL(0, ops->Load(ops));
}
