#include <cstring>

#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogTimestamp.h"
#include "SolidSyslogTimestampFormatter.h"

enum
{
    TEST_BUFFER_SIZE = 64
};

#define CHECK_FORMATTED(expected)                                              \
    STRCMP_EQUAL(expected, SolidSyslogFormatter_AsFormattedBuffer(formatter)); \
    LONGS_EQUAL(strlen(expected), SolidSyslogFormatter_Length(formatter))

TEST_GROUP(SolidSyslogTimestampFormatter)
{
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(TEST_BUFFER_SIZE)];
    struct SolidSyslogFormatter* formatter;

    void setup() override { formatter = SolidSyslogFormatter_Create(storage, TEST_BUFFER_SIZE); }
};

TEST(SolidSyslogTimestampFormatter, FormatsValidTimestampAsRfc3339WithZuluOffset)
{
    struct SolidSyslogTimestamp ts = {2026, 4, 2, 14, 30, 7, 42, 0};

    SolidSyslogTimestampFormatter_Format(formatter, &ts);

    CHECK_FORMATTED("2026-04-02T14:30:07.000042Z");
}
