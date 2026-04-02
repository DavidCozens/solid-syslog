#include "CppUTest/TestHarness.h"
#include "SolidSyslogPosixClock.h"
#include "ClockFake.h"

// 2025-04-02T00:00:00Z
static const time_t TEST_EPOCH = 1743552000;

TEST_GROUP(SolidSyslogPosixClock){void setup() override{ClockFake_Reset();
ClockFake_SetTime(TEST_EPOCH, 0);
}
}
;

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
