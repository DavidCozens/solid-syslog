#include "CppUTest/TestHarness.h"
#include "SolidSyslogAtomicCounter.h"

#include <cstdlib>

// clang-format off
TEST_GROUP(SolidSyslogAtomicCounter)
{
    SolidSyslogAtomicCounter* counter;

    void setup() override
    {
        counter = SolidSyslogAtomicCounter_Create(malloc);
    }

    void teardown() override
    {
        SolidSyslogAtomicCounter_Destroy(counter, free);
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
