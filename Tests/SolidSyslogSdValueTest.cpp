#include <cstring>

#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogSdValue.h"
#include "SolidSyslogSdValuePrivate.h"

enum
{
    TEST_BUFFER_SIZE = 64
};

#define CHECK_VALUE(expected) STRCMP_EQUAL(expected, SolidSyslogFormatter_AsFormattedBuffer(formatter))

// clang-format off
TEST_GROUP(SolidSyslogSdValue)
{
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(TEST_BUFFER_SIZE)];
    struct SolidSyslogFormatter* formatter = nullptr;
    struct SolidSyslogSdValue value;

    void setup() override
    {
        formatter = SolidSyslogFormatter_Create(storage, TEST_BUFFER_SIZE);
        SolidSyslogSdValue_FromFormatter(&value, formatter);
    }

    void writeString(const char* source) { SolidSyslogSdValue_String(&value, source); }
};

// clang-format on

TEST(SolidSyslogSdValue, StringPassesPlainAsciiThrough)
{
    writeString("hello");

    CHECK_VALUE("hello");
}
