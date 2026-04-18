#include "CppUTest/TestHarness.h"
#include "ExampleAppName.h"
#include "SolidSyslogFormatter.h"

enum
{
    FORMATTER_BUFFER_SIZE = 64
};

// clang-format off
TEST_GROUP(ExampleAppName)
{
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(FORMATTER_BUFFER_SIZE)];
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogFormatter* formatter = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        formatter = SolidSyslogFormatter_Create(storage, FORMATTER_BUFFER_SIZE);
    }

    [[nodiscard]] const char* formatted() const
    {
        return SolidSyslogFormatter_AsString(formatter);
    }
};

// clang-format on

TEST(ExampleAppName, BackslashSeparatorExtractsBaseName)
{
    ExampleAppName_Set("dir\\binary.exe");
    ExampleAppName_Get(formatter);
    STRCMP_EQUAL("binary.exe", formatted());
}

TEST(ExampleAppName, ForwardSlashSeparatorExtractsBaseName)
{
    ExampleAppName_Set("/usr/local/bin/example");
    ExampleAppName_Get(formatter);
    STRCMP_EQUAL("example", formatted());
}

TEST(ExampleAppName, NoSeparatorReturnsWholeArgument)
{
    ExampleAppName_Set("example");
    ExampleAppName_Get(formatter);
    STRCMP_EQUAL("example", formatted());
}

TEST(ExampleAppName, MixedSeparatorsUseRightmost)
{
    ExampleAppName_Set("C:\\msys64\\home/user/example.exe");
    ExampleAppName_Get(formatter);
    STRCMP_EQUAL("example.exe", formatted());
}
