#include "CppUTest/TestHarness.h"
#include "SolidSyslogPosixClock.h"
#include "ClockFake.h"

// 2025-04-02T00:00:00Z
static const time_t TEST_EPOCH = 1743552000;

// clang-format off
TEST_GROUP(SolidSyslogPosixClock)
{
    void setup() override
    {
        ClockFake_Reset();
        ClockFake_SetTime(TEST_EPOCH, 0);
    }
};

// clang-format on

TEST(SolidSyslogPosixClock, YearMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(2025, ts.year);
}

TEST(SolidSyslogPosixClock, MonthMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(4, ts.month);
}

TEST(SolidSyslogPosixClock, DayMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(2, ts.day);
}

TEST(SolidSyslogPosixClock, HourMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.hour);
}

TEST(SolidSyslogPosixClock, MinuteMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.minute);
}

TEST(SolidSyslogPosixClock, SecondMatchesKnownTime)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.second);
}

TEST(SolidSyslogPosixClock, MicrosecondFromNanoseconds)
{
    ClockFake_SetTime(TEST_EPOCH, 123456789);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(123456, ts.microsecond);
}

TEST(SolidSyslogPosixClock, UtcOffsetIsAlwaysZero)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.utcOffsetMinutes);
}

TEST(SolidSyslogPosixClock, ClockGettimeFailureReturnsInvalidTimestamp)
{
    ClockFake_SetClockGettimeReturn(-1);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.month);
}

TEST(SolidSyslogPosixClock, GmtimeFailureReturnsInvalidTimestamp)
{
    ClockFake_SetGmtimeReturn(nullptr);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.month);
}

// 2025-01-01T00:00:00Z — January (month 1)
TEST(SolidSyslogPosixClock, Month1FromJanuaryEpoch)
{
    ClockFake_SetTime(1735689600, 0);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(1, ts.month);
}

// 2025-12-31T00:00:00Z — December (month 12), day 31
TEST(SolidSyslogPosixClock, Month12Day31FromDecemberEpoch)
{
    ClockFake_SetTime(1767139200, 0);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(12, ts.month);
    LONGS_EQUAL(31, ts.day);
}

// 2025-04-01T00:00:00Z — day 1
TEST(SolidSyslogPosixClock, Day1FromFirstOfMonth)
{
    ClockFake_SetTime(1743465600, 0);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(1, ts.day);
}

// 2025-04-02T23:59:59Z — hour 23, minute 59, second 59
TEST(SolidSyslogPosixClock, Hour23Minute59Second59)
{
    ClockFake_SetTime(1743638399, 0);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(23, ts.hour);
    LONGS_EQUAL(59, ts.minute);
    LONGS_EQUAL(59, ts.second);
}

// Nanoseconds 999999999 truncated to microsecond 999999
TEST(SolidSyslogPosixClock, MaxNanosecondsProducesMaxMicroseconds)
{
    ClockFake_SetTime(TEST_EPOCH, 999999999);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(999999, ts.microsecond);
}

// Nanoseconds 0 produces microsecond 0
TEST(SolidSyslogPosixClock, ZeroNanosecondsProducesZeroMicroseconds)
{
    ClockFake_SetTime(TEST_EPOCH, 0);
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    LONGS_EQUAL(0, ts.microsecond);
}

TEST(SolidSyslogPosixClock, AllFieldsInValidRanges)
{
    struct SolidSyslogTimestamp ts = SolidSyslogPosixClock_GetTimestamp();
    CHECK(ts.year > 0);
    CHECK(ts.month >= 1 && ts.month <= 12);
    CHECK(ts.day >= 1 && ts.day <= 31);
    CHECK(ts.hour <= 23);
    CHECK(ts.minute <= 59);
    CHECK(ts.second <= 59);
    CHECK(ts.microsecond <= 999999);
    LONGS_EQUAL(0, ts.utcOffsetMinutes);
}
