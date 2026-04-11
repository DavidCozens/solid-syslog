#include "CppUTest/TestHarness.h"
#include "FileFake.h"

#include <cstring>

// clang-format off
TEST_GROUP(FileFake)
{
    struct SolidSyslogFileApi* api = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        api = FileFake_Create();
    }

    void teardown() override
    {
        FileFake_Destroy();
    }
};

// clang-format on

TEST(FileFake, CreateReturnsNonNull)
{
    CHECK_TRUE(api != nullptr);
}

TEST(FileFake, IsOpenReturnsFalseBeforeOpen)
{
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(api));
}

TEST(FileFake, OpenSetsIsOpen)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    CHECK_TRUE(SolidSyslogFileApi_IsOpen(api));
}

TEST(FileFake, CloseClearsIsOpen)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Close(api);
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(api));
}

TEST(FileFake, WriteAndReadRoundTrip)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_SeekTo(api, 0);

    char buf[16] = {};
    CHECK_TRUE(SolidSyslogFileApi_Read(api, buf, 5));
    MEMCMP_EQUAL("hello", buf, 5);
}

TEST(FileFake, ReadBeyondFileSizeFails)
{
    SolidSyslogFileApi_Open(api, "test.dat");

    char buf[16];
    CHECK_FALSE(SolidSyslogFileApi_Read(api, buf, 1));
}

TEST(FileFake, SeekToChangesReadPosition)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "abcde", 5);
    SolidSyslogFileApi_SeekTo(api, 3);

    char buf[16] = {};
    SolidSyslogFileApi_Read(api, buf, 2);
    MEMCMP_EQUAL("de", buf, 2);
}

TEST(FileFake, SizeReturnsFileSize)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    LONGS_EQUAL(0, SolidSyslogFileApi_Size(api));
    SolidSyslogFileApi_Write(api, "hello", 5);
    LONGS_EQUAL(5, SolidSyslogFileApi_Size(api));
}

TEST(FileFake, TruncateClearsContent)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_Truncate(api);
    LONGS_EQUAL(0, SolidSyslogFileApi_Size(api));
}

TEST(FileFake, WriteAtPositionOverwritesContent)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_SeekTo(api, 0);
    SolidSyslogFileApi_Write(api, "HE", 2);
    SolidSyslogFileApi_SeekTo(api, 0);

    char buf[16] = {};
    SolidSyslogFileApi_Read(api, buf, 5);
    MEMCMP_EQUAL("HEllo", buf, 5);
}

TEST(FileFake, FailNextOpenMakesOpenReturnFalse)
{
    FileFake_FailNextOpen();
    CHECK_FALSE(SolidSyslogFileApi_Open(api, "test.dat"));
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(api));
}

TEST(FileFake, FailNextOpenOnlyAffectsOneCall)
{
    FileFake_FailNextOpen();
    SolidSyslogFileApi_Open(api, "test.dat");
    CHECK_TRUE(SolidSyslogFileApi_Open(api, "test.dat"));
}

TEST(FileFake, FailNextWriteMakesWriteReturnFalse)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    FileFake_FailNextWrite();
    CHECK_FALSE(SolidSyslogFileApi_Write(api, "hello", 5));
}

TEST(FileFake, FailNextWriteOnlyAffectsOneCall)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    FileFake_FailNextWrite();
    SolidSyslogFileApi_Write(api, "hello", 5);
    CHECK_TRUE(SolidSyslogFileApi_Write(api, "world", 5));
}

TEST(FileFake, FailNextReadMakesReadReturnFalse)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_SeekTo(api, 0);
    FileFake_FailNextRead();

    char buf[16];
    CHECK_FALSE(SolidSyslogFileApi_Read(api, buf, 5));
}

TEST(FileFake, FailNextReadOnlyAffectsOneCall)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_SeekTo(api, 0);
    FileFake_FailNextRead();

    char buf[16] = {};
    SolidSyslogFileApi_Read(api, buf, 5);
    SolidSyslogFileApi_SeekTo(api, 0);
    CHECK_TRUE(SolidSyslogFileApi_Read(api, buf, 5));
}

TEST(FileFake, FileContentReturnsInternalBuffer)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    MEMCMP_EQUAL("hello", FileFake_FileContent(), 5);
}

TEST(FileFake, FileSizeMatchesApiSize)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    LONGS_EQUAL(SolidSyslogFileApi_Size(api), FileFake_FileSize());
}

TEST(FileFake, OpenPreservesExistingContent)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Write(api, "hello", 5);
    SolidSyslogFileApi_Close(api);
    SolidSyslogFileApi_Open(api, "test.dat");

    LONGS_EQUAL(5, SolidSyslogFileApi_Size(api));
    char buf[16] = {};
    SolidSyslogFileApi_Read(api, buf, 5);
    MEMCMP_EQUAL("hello", buf, 5);
}

TEST(FileFake, ExistsReturnsTrueForOpenFile)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    CHECK_TRUE(SolidSyslogFileApi_Exists(api, "test.dat"));
}

TEST(FileFake, ExistsReturnsFalseForUnknownFile)
{
    CHECK_FALSE(SolidSyslogFileApi_Exists(api, "unknown.dat"));
}

TEST(FileFake, ExistsReturnsTrueForClosedFile)
{
    SolidSyslogFileApi_Open(api, "test.dat");
    SolidSyslogFileApi_Close(api);
    CHECK_TRUE(SolidSyslogFileApi_Exists(api, "test.dat"));
}

TEST(FileFake, CanOpenTwoFilesByName)
{
    SolidSyslogFileApi_Open(api, "a.log");
    SolidSyslogFileApi_Write(api, "aaa", 3);
    SolidSyslogFileApi_Close(api);

    SolidSyslogFileApi_Open(api, "b.log");
    SolidSyslogFileApi_Write(api, "bbb", 3);
    SolidSyslogFileApi_Close(api);

    SolidSyslogFileApi_Open(api, "a.log");
    char buf[16] = {};
    SolidSyslogFileApi_Read(api, buf, 3);
    MEMCMP_EQUAL("aaa", buf, 3);
}

TEST(FileFake, ExistsDistinguishesFilesByName)
{
    SolidSyslogFileApi_Open(api, "a.log");
    SolidSyslogFileApi_Close(api);

    CHECK_TRUE(SolidSyslogFileApi_Exists(api, "a.log"));
    CHECK_FALSE(SolidSyslogFileApi_Exists(api, "b.log"));
}
