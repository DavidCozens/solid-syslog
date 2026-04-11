#include "CppUTest/TestHarness.h"
#include "SolidSyslogFileStore.h"
#include "FileFake.h"

#include <cstring>

static const char* const TEST_PATH_PREFIX = "/tmp/test_store";
static const char* const TEST_DATA        = "hello";
static const size_t      TEST_DATA_LEN    = 5;

enum
{
    TEST_BUF_SIZE      = 512,
    SENTINEL           = 'Z',
    TEST_MAX_FILE_SIZE = 4096,
    TEST_MAX_FILES     = 2
};

static const struct SolidSyslogFileStoreConfig DEFAULT_CONFIG = {
    nullptr, nullptr, TEST_PATH_PREFIX, TEST_MAX_FILE_SIZE, TEST_MAX_FILES, SOLIDSYSLOG_DISCARD_OLDEST,
};

static struct SolidSyslogFileStoreConfig MakeConfig(struct SolidSyslogFile* file)
{
    struct SolidSyslogFileStoreConfig config = DEFAULT_CONFIG;
    config.readFile                          = file;
    config.writeFile                         = file;
    return config;
}

/* ------------------------------------------------------------------
 * Basic operations
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStore)
{
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create();
        struct SolidSyslogFileStoreConfig config = MakeConfig(file);
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogFileStore, CreateReturnsNonNull)
{
    CHECK_TRUE(store != nullptr);
}

TEST(SolidSyslogFileStore, CreatesFileWithSequence00)
{
    CHECK_TRUE(SolidSyslogFile_Exists(file, "/tmp/test_store00.log"));
}

TEST(SolidSyslogFileStore, HasUnsentReturnsFalseOnEmpty)
{
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStore, WriteReturnsTrue)
{
    CHECK_TRUE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
}

TEST(SolidSyslogFileStore, HasUnsentReturnsTrueAfterWrite)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStore, ReadNextUnsentReturnsTrueAfterWrite)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    CHECK_TRUE(SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead));
}

TEST(SolidSyslogFileStore, ReadNextUnsentReturnsWrittenData)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL(TEST_DATA, buf, TEST_DATA_LEN);
}

TEST(SolidSyslogFileStore, ReadNextUnsentReturnsByteCount)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    LONGS_EQUAL(TEST_DATA_LEN, bytesRead);
}

TEST(SolidSyslogFileStore, ReadNextUnsentReturnsFalseOnEmpty)
{
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    CHECK_FALSE(SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead));
}

TEST(SolidSyslogFileStore, ReadNextUnsentSetsZeroBytesOnEmpty)
{
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 99;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    LONGS_EQUAL(0, bytesRead);
}

TEST(SolidSyslogFileStore, ReadDoesNotWriteBeyondDataLength)
{
    char buf[TEST_BUF_SIZE];
    memset(buf, SENTINEL, sizeof(buf));

    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    MEMCMP_EQUAL(TEST_DATA, buf, TEST_DATA_LEN);
    BYTES_EQUAL(SENTINEL, buf[TEST_DATA_LEN]);
}

TEST(SolidSyslogFileStore, ReadTruncatesWhenBufferTooSmall)
{
    const char* longMessage = "hello world";

    enum
    {
        SMALL_BUF_SIZE = 5
    };

    SolidSyslogStore_Write(store, longMessage, strlen(longMessage));

    char   buf[SMALL_BUF_SIZE] = {};
    size_t bytesRead           = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    LONGS_EQUAL(SMALL_BUF_SIZE, bytesRead);
    MEMCMP_EQUAL("hello", buf, SMALL_BUF_SIZE);
}

TEST(SolidSyslogFileStore, MarkSentThenHasUnsentReturnsFalse)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStore, MarkSentWithoutReadDoesNotCrash)
{
    SolidSyslogStore_MarkSent(store);
}

TEST(SolidSyslogFileStore, FileTruncatedAfterAllSent)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    LONGS_EQUAL(0, FileFake_FileSize());
}

TEST(SolidSyslogFileStore, WriteAfterDrainWorks)
{
    SolidSyslogStore_Write(store, "first", strlen("first"));
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    SolidSyslogStore_Write(store, "second", strlen("second"));
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("second", buf, strlen("second"));
}

TEST(SolidSyslogFileStore, TwoWritesFirstReadReturnsFirst)
{
    SolidSyslogStore_Write(store, "first", strlen("first"));
    SolidSyslogStore_Write(store, "second", strlen("second"));
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("first", buf, strlen("first"));
}

TEST(SolidSyslogFileStore, AfterMarkFirstReadReturnsSecond)
{
    SolidSyslogStore_Write(store, "first", strlen("first"));
    SolidSyslogStore_Write(store, "second", strlen("second"));
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("second", buf, strlen("second"));
    LONGS_EQUAL(strlen("second"), bytesRead);
}

TEST(SolidSyslogFileStore, FiveWritesDrainAllInOrder)
{
    const char* messages[] = {"msg0", "msg1", "msg2", "msg3", "msg4"};

    enum
    {
        MESSAGE_LEN = 4
    };

    for (const auto* msg : messages)
    {
        SolidSyslogStore_Write(store, msg, MESSAGE_LEN);
    }

    for (const auto* expected : messages)
    {
        char   buf[TEST_BUF_SIZE] = {};
        size_t bytesRead          = 0;
        CHECK_TRUE(SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead));
        MEMCMP_EQUAL(expected, buf, MESSAGE_LEN);
        SolidSyslogStore_MarkSent(store);
    }

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

/* ------------------------------------------------------------------
 * Resume from existing file
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreResume)
{
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        file = FileFake_Create();
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- total and markedSent have distinct semantics
    void WritePreviousSession(int total, int markedSent)
    {
        struct SolidSyslogFileStoreConfig config = MakeConfig(file);
        // cppcheck-suppress unreadVariable -- used by WriteMessages/DrainMessages; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
        WriteMessages(total);
        DrainMessages(markedSent);
        SolidSyslogFileStore_Destroy();

        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void WriteMessages(int count) const
    {
        for (int i = 0; i < count; i++)
        {
            char msg[16];
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) -- snprintf is the bounded C formatting API
            snprintf(msg, sizeof(msg), "msg%d", i);
            SolidSyslogStore_Write(store, msg, strlen(msg));
        }
    }

    void DrainMessages(int count) const
    {
        char   buf[TEST_BUF_SIZE];
        size_t bytesRead = 0;
        for (int i = 0; i < count; i++)
        {
            SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
            SolidSyslogStore_MarkSent(store);
        }
    }
};

// clang-format on

TEST(SolidSyslogFileStoreResume, HasUnsentAfterResume)
{
    WritePreviousSession(3, 1);
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreResume, ReadReturnsFirstUnsent)
{
    WritePreviousSession(3, 1);
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("msg1", buf, strlen("msg1"));
}

TEST(SolidSyslogFileStoreResume, DrainsRemainingUnsent)
{
    WritePreviousSession(3, 1);
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("msg1", buf, strlen("msg1"));
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("msg2", buf, strlen("msg2"));
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreResume, AllSentReturnsNoUnsent)
{
    WritePreviousSession(3, 3);
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreResume, EmptyFileReturnsNoUnsent)
{
    WritePreviousSession(0, 0);
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreResume, CanWriteNewMessagesAfterResume)
{
    WritePreviousSession(2, 1);
    char   buf[TEST_BUF_SIZE] = {};
    size_t bytesRead          = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    SolidSyslogStore_Write(store, "new", strlen("new"));
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    MEMCMP_EQUAL("new", buf, strlen("new"));
}

/* ------------------------------------------------------------------
 * Destroy
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreDestroy)
{
    struct SolidSyslogFile* file = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create();
    }

    void teardown() override
    {
        FileFake_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogFileStoreDestroy, DestroyClosesFile)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    SolidSyslogFileStore_Create(&config);
    CHECK_TRUE(SolidSyslogFile_IsOpen(file));
    SolidSyslogFileStore_Destroy();
    CHECK_FALSE(SolidSyslogFile_IsOpen(file));
}

TEST(SolidSyslogFileStoreDestroy, DoubleDestroyDoesNotCrash)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    SolidSyslogFileStore_Create(&config);
    SolidSyslogFileStore_Destroy();
    SolidSyslogFileStore_Destroy();
}

/* ------------------------------------------------------------------
 * Config validation
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreConfig)
{
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create();
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }

    void CreateWithMaxFiles(size_t maxFiles)
    {
        struct SolidSyslogFileStoreConfig config = MakeConfig(file);
        config.maxFiles = maxFiles;
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void CreateWithMaxFileSize(size_t maxFileSize)
    {
        struct SolidSyslogFileStoreConfig config = MakeConfig(file);
        config.maxFileSize = maxFileSize;
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void CreateWithPathPrefix(const char* prefix)
    {
        struct SolidSyslogFileStoreConfig config = MakeConfig(file);
        config.pathPrefix = prefix;
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void VerifyWriteAndReadBack() const
    {
        CHECK_TRUE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
        char   buf[TEST_BUF_SIZE] = {};
        size_t bytesRead = 0;
        SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
        MEMCMP_EQUAL(TEST_DATA, buf, TEST_DATA_LEN);
    }
};

// clang-format on

TEST(SolidSyslogFileStoreConfig, MaxFilesZeroClampedToMinimum)
{
    CreateWithMaxFiles(0);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, MaxFilesOneClampedToMinimum)
{
    CreateWithMaxFiles(1);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, MaxFilesHundredClampedToMaximum)
{
    CreateWithMaxFiles(100);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, MaxFileSizeZeroClampedToMinimum)
{
    CreateWithMaxFileSize(0);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, MaxFileSizeOneClampedToMinimum)
{
    CreateWithMaxFileSize(1);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, FilenameExactlyAtMaxPath)
{
    /* MAX_PATH_SIZE=128, suffix "00.log"=6, null=1, so max prefix=121 chars */
    char prefix[122];
    memset(prefix, 'A', 121);
    prefix[121] = '\0';

    CreateWithPathPrefix(prefix);

    char expected[129];
    memset(expected, 'A', 121);
    memcpy(expected + 121, "00.log", 7);

    CHECK_TRUE(SolidSyslogFile_Exists(file, expected));
}

TEST(SolidSyslogFileStoreConfig, FilenameTruncatedWhenPrefixTooLong)
{
    /* Prefix longer than max — filename is truncated but store still works */
    char prefix[140];
    memset(prefix, 'B', 139);
    prefix[139] = '\0';

    CreateWithPathPrefix(prefix);
    VerifyWriteAndReadBack();
}

/* ------------------------------------------------------------------
 * Error paths
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreErrors)
{
    struct SolidSyslogFile* file = nullptr;
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create();
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogFileStoreErrors, OpenFailureStillReturnsNonNull)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(&config);
    CHECK_TRUE(store != nullptr);
}

TEST(SolidSyslogFileStoreErrors, WriteReturnsFalseWhenNotOpen)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(&config);
    CHECK_FALSE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
}

TEST(SolidSyslogFileStoreErrors, WriteReturnsFalseOnWriteFailure)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    store                                    = SolidSyslogFileStore_Create(&config);
    FileFake_FailNextWrite();
    CHECK_FALSE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
}

TEST(SolidSyslogFileStoreErrors, ReadReturnsFalseOnReadFailure)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    store                                    = SolidSyslogFileStore_Create(&config);
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    FileFake_FailNextRead();

    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    CHECK_FALSE(SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead));
    LONGS_EQUAL(0, bytesRead);
}

TEST(SolidSyslogFileStoreErrors, HasUnsentReturnsFalseWhenNotOpen)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(&config);
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreErrors, MarkSentDoesNotAdvanceWhenWriteFails)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    store                                    = SolidSyslogFileStore_Create(&config);
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);

    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    FileFake_FailNextWrite();
    SolidSyslogStore_MarkSent(store);

    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}
