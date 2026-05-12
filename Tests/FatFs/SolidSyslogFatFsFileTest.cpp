#include "CppUTest/TestHarness.h"
#include "TestUtils.h"

extern "C"
{
#include "FatFsFake.h"
#include "SolidSyslogFatFsFile.h"
#include "SolidSyslogFile.h"
#include "ff.h"
}

using namespace CososoTesting;

static const char* const TEST_PATH = "test.log";

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- macros preserve __FILE__/__LINE__ in test failure output
#define CHECK_FILE_IS_OPEN() CHECK_TRUE(SolidSyslogFile_IsOpen(file))
#define CHECK_FILE_CLOSED() CHECK_FALSE(SolidSyslogFile_IsOpen(file))
#define CHECK_OPEN_PATH(path) STRCMP_EQUAL((path), FatFsFake_LastOpenPath())
#define CHECK_OPEN_MODE(mode) LONGS_EQUAL((mode), FatFsFake_LastOpenMode())
#define CHECK_LSEEK_OFFSET(offset) LONGS_EQUAL((offset), FatFsFake_LastLseekOffset())
#define CHECK_READ_BUF(buf) POINTERS_EQUAL((buf), FatFsFake_LastReadBuf())
#define CHECK_READ_COUNT(count) LONGS_EQUAL((count), FatFsFake_LastReadCount())
#define CHECK_WRITE_BUF(buf) POINTERS_EQUAL((buf), FatFsFake_LastWriteBuf())
#define CHECK_WRITE_COUNT(count) LONGS_EQUAL((count), FatFsFake_LastWriteCount())
#define CHECK_STAT_PATH(path) STRCMP_EQUAL((path), FatFsFake_LastStatPath())
#define CHECK_UNLINK_PATH(path) STRCMP_EQUAL((path), FatFsFake_LastUnlinkPath())

// NOLINTEND(cppcoreguidelines-macro-usage)

// clang-format off
TEST_GROUP(SolidSyslogFatFsFile)
{
    SolidSyslogFatFsFileStorage storage = {};
    struct SolidSyslogFile*     file    = nullptr;

    void setup() override
    {
        FatFsFake_Reset();
        file = SolidSyslogFatFsFile_Create(&storage);
    }

    void teardown() override
    {
        SolidSyslogFatFsFile_Destroy(file);
    }

    void Open() const                 { CHECK_TRUE(SolidSyslogFile_Open(file, TEST_PATH)); }
    void Open(const char* path) const { CHECK_TRUE(SolidSyslogFile_Open(file, path)); }
    void Close() const                { SolidSyslogFile_Close(file); }
};

// clang-format on

TEST(SolidSyslogFatFsFile, CreateSucceeds)
{
    CHECK(file != nullptr);
}

TEST(SolidSyslogFatFsFile, IsOpenIsFalseAfterCreate)
{
    CHECK_FILE_CLOSED();
}

TEST(SolidSyslogFatFsFile, OpenSucceeds)
{
    CHECK_TRUE(SolidSyslogFile_Open(file, TEST_PATH));
    CHECK_FILE_IS_OPEN();
}

TEST(SolidSyslogFatFsFile, OpenCallsFOpenWithCorrectDefaults)
{
    Open();
    CALLED_FAKE(FatFsFake_Open, ONCE);
    CHECK_OPEN_PATH(TEST_PATH);
    CHECK_OPEN_MODE(FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
}

TEST(SolidSyslogFatFsFile, OpenUsesPassedFilename)
{
    Open("different.log");
    CHECK_OPEN_PATH("different.log");
}

TEST(SolidSyslogFatFsFile, OpenFailsWhenFOpenFails)
{
    FatFsFake_SetOpenResult(FR_NO_PATH);
    CHECK_FALSE(SolidSyslogFile_Open(file, TEST_PATH));
    CHECK_FILE_CLOSED();
}

TEST(SolidSyslogFatFsFile, CloseCallsFCloseAndClearsIsOpen)
{
    Open();
    Close();
    CALLED_FAKE(FatFsFake_Close, ONCE);
    CHECK_FILE_CLOSED();
}

TEST(SolidSyslogFatFsFile, CloseIsNoOpWhenAlreadyClosed)
{
    Close();
    CALLED_FAKE(FatFsFake_Close, NEVER);
}

TEST(SolidSyslogFatFsFile, DestroyClosesOpenFile)
{
    SolidSyslogFatFsFileStorage localStorage = {};
    struct SolidSyslogFile*     localFile    = SolidSyslogFatFsFile_Create(&localStorage);
    SolidSyslogFile_Open(localFile, TEST_PATH);
    SolidSyslogFatFsFile_Destroy(localFile);
    CALLED_FAKE(FatFsFake_Close, ONCE);
}

TEST(SolidSyslogFatFsFile, TruncateSeeksToZeroAndCallsFTruncate)
{
    Open();
    SolidSyslogFile_Truncate(file);
    CALLED_FAKE(FatFsFake_Lseek, ONCE);
    CHECK_LSEEK_OFFSET(0);
    CALLED_FAKE(FatFsFake_Truncate, ONCE);
}

TEST(SolidSyslogFatFsFile, SeekToCallsFLseekWithGivenOffset)
{
    Open();
    SolidSyslogFile_SeekTo(file, 42);
    CALLED_FAKE(FatFsFake_Lseek, ONCE);
    CHECK_LSEEK_OFFSET(42);

    SolidSyslogFile_SeekTo(file, 99);
    CHECK_LSEEK_OFFSET(99);
}

TEST(SolidSyslogFatFsFile, SizeReturnsFileObjectSize)
{
    Open();
    FatFsFake_SetFileSize(42);
    LONGS_EQUAL(42, SolidSyslogFile_Size(file));
}

TEST(SolidSyslogFatFsFile, ReadCallsFReadWithCorrectDefaults)
{
    Open();
    char buf[5] = {};
    CHECK_TRUE(SolidSyslogFile_Read(file, buf, sizeof(buf)));
    CALLED_FAKE(FatFsFake_Read, ONCE);
    CHECK_READ_BUF(buf);
    CHECK_READ_COUNT(sizeof(buf));
}

TEST(SolidSyslogFatFsFile, ReadFailsWhenFReadReturnsPartial)
{
    Open();
    FatFsFake_SetReadBytesReturned(3);
    char buf[5];
    CHECK_FALSE(SolidSyslogFile_Read(file, buf, sizeof(buf)));
}

TEST(SolidSyslogFatFsFile, ReadFailsWhenFReadFails)
{
    Open();
    FatFsFake_SetReadResult(FR_DISK_ERR);
    char buf[5];
    CHECK_FALSE(SolidSyslogFile_Read(file, buf, sizeof(buf)));
}

TEST(SolidSyslogFatFsFile, WriteCallsFWriteWithCorrectDefaults)
{
    Open();
    const char buf[5] = {'h', 'e', 'l', 'l', 'o'};
    CHECK_TRUE(SolidSyslogFile_Write(file, buf, sizeof(buf)));
    CALLED_FAKE(FatFsFake_Write, ONCE);
    CHECK_WRITE_BUF(buf);
    CHECK_WRITE_COUNT(sizeof(buf));
}

TEST(SolidSyslogFatFsFile, WriteFailsWhenFWriteReturnsPartial)
{
    Open();
    FatFsFake_SetWriteBytesReturned(3);
    const char buf[5] = {};
    CHECK_FALSE(SolidSyslogFile_Write(file, buf, sizeof(buf)));
}

TEST(SolidSyslogFatFsFile, WriteFailsWhenFWriteFails)
{
    Open();
    FatFsFake_SetWriteResult(FR_DISK_ERR);
    const char buf[5] = {};
    CHECK_FALSE(SolidSyslogFile_Write(file, buf, sizeof(buf)));
}

TEST(SolidSyslogFatFsFile, ExistsCallsFStatAndReportsTrue)
{
    CHECK_TRUE(SolidSyslogFile_Exists(file, TEST_PATH));
    CALLED_FAKE(FatFsFake_Stat, ONCE);
    CHECK_STAT_PATH(TEST_PATH);
}

TEST(SolidSyslogFatFsFile, ExistsUsesPassedPath)
{
    SolidSyslogFile_Exists(file, "different.log");
    CHECK_STAT_PATH("different.log");
}

TEST(SolidSyslogFatFsFile, ExistsReportsFalseWhenFStatFails)
{
    FatFsFake_SetStatResult(FR_NO_FILE);
    CHECK_FALSE(SolidSyslogFile_Exists(file, TEST_PATH));
}

TEST(SolidSyslogFatFsFile, DeleteCallsFUnlinkAndReportsTrue)
{
    CHECK_TRUE(SolidSyslogFile_Delete(file, TEST_PATH));
    CALLED_FAKE(FatFsFake_Unlink, ONCE);
    CHECK_UNLINK_PATH(TEST_PATH);
}

TEST(SolidSyslogFatFsFile, DeleteUsesPassedPath)
{
    SolidSyslogFile_Delete(file, "different.log");
    CHECK_UNLINK_PATH("different.log");
}

TEST(SolidSyslogFatFsFile, DeleteReportsFalseWhenFUnlinkFails)
{
    FatFsFake_SetUnlinkResult(FR_DENIED);
    CHECK_FALSE(SolidSyslogFile_Delete(file, TEST_PATH));
}
