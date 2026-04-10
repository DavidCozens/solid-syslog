#include "CppUTest/TestHarness.h"
#include "SolidSyslogPosixFileApi.h"
#include "SocketFake.h"

#include <cstdio>

static const char* const TEST_PATH = "/tmp/test_posix_file_api.dat";

// clang-format off
TEST_GROUP(SolidSyslogPosixFileApi)
{
    struct SolidSyslogFileApi* api = nullptr;

    void setup() override
    {
        SocketFake_Reset();
        remove(TEST_PATH);
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        api = SolidSyslogPosixFileApi_Create();
    }

    void teardown() override
    {
        SolidSyslogPosixFileApi_Destroy();
        remove(TEST_PATH);
    }
};

// clang-format on

TEST(SolidSyslogPosixFileApi, CreateReturnsNonNull)
{
    CHECK_TRUE(api != nullptr);
}

TEST(SolidSyslogPosixFileApi, IsOpenReturnsFalseBeforeOpen)
{
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(api));
}

TEST(SolidSyslogPosixFileApi, OpenReturnsTrue)
{
    CHECK_TRUE(SolidSyslogFileApi_Open(api, TEST_PATH));
}

TEST(SolidSyslogPosixFileApi, OpenSetsIsOpen)
{
    SolidSyslogFileApi_Open(api, TEST_PATH);
    CHECK_TRUE(SolidSyslogFileApi_IsOpen(api));
}

TEST(SolidSyslogPosixFileApi, CloseResetsIsOpen)
{
    SolidSyslogFileApi_Open(api, TEST_PATH);
    SolidSyslogFileApi_Close(api);
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(api));
}

TEST(SolidSyslogPosixFileApi, WriteAndReadRoundTrip)
{
    SolidSyslogFileApi_Open(api, TEST_PATH);
    CHECK_TRUE(SolidSyslogFileApi_Write(api, "hello", 5));
    SolidSyslogFileApi_SeekTo(api, 0);

    char buf[16] = {};
    CHECK_TRUE(SolidSyslogFileApi_Read(api, buf, 5));
    MEMCMP_EQUAL("hello", buf, 5);
}

TEST(SolidSyslogPosixFileApi, SizeReturnsFileSize)
{
    SolidSyslogFileApi_Open(api, TEST_PATH);
    LONGS_EQUAL(0, SolidSyslogFileApi_Size(api));
    SolidSyslogFileApi_Write(api, "hello", 5);
    LONGS_EQUAL(5, SolidSyslogFileApi_Size(api));
}

TEST(SolidSyslogPosixFileApi, TruncateClearsFile)
{
    SolidSyslogFileApi_Open(api, TEST_PATH);
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_Truncate(api);
    LONGS_EQUAL(0, SolidSyslogFileApi_Size(api));
}

TEST(SolidSyslogPosixFileApi, OpenWithInvalidPathReturnsFalse)
{
    CHECK_FALSE(SolidSyslogFileApi_Open(api, "/nonexistent/dir/file.dat"));
}
