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
    STRCMP_EQUAL(TEST_PATH, FatFsFake_LastOpenPath());
    LONGS_EQUAL(FA_READ | FA_WRITE | FA_OPEN_ALWAYS, FatFsFake_LastOpenMode());
}

TEST(SolidSyslogFatFsFile, OpenUsesPassedFilename)
{
    Open("different.log");
    STRCMP_EQUAL("different.log", FatFsFake_LastOpenPath());
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
