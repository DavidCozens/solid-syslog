#include "CppUTest/TestHarness.h"
#include "SolidSyslogAtomicCounter.h"

// clang-format off
TEST_GROUP(SolidSyslogAtomicCounter)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogAtomicCounter* counter;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        counter = SolidSyslogAtomicCounter_Create();
    }

    void teardown() override
    {
        SolidSyslogAtomicCounter_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogAtomicCounter, CreateReturnsNonNull)
{
    CHECK(counter != nullptr);
}

TEST(SolidSyslogAtomicCounter, FirstIncrementReturns1)
{
    LONGS_EQUAL(1, SolidSyslogAtomicCounter_Increment(counter));
}

TEST(SolidSyslogAtomicCounter, SecondIncrementReturns2)
{
    SolidSyslogAtomicCounter_Increment(counter);
    LONGS_EQUAL(2, SolidSyslogAtomicCounter_Increment(counter));
}

TEST(SolidSyslogAtomicCounter, ThirdIncrementReturns3)
{
    SolidSyslogAtomicCounter_Increment(counter);
    SolidSyslogAtomicCounter_Increment(counter);
    LONGS_EQUAL(3, SolidSyslogAtomicCounter_Increment(counter));
}
