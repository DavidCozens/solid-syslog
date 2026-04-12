#include "CppUTest/TestHarness.h"
#include "SolidSyslogFileStore.h"
#include "SolidSyslogSecurityPolicyDefinition.h"
#include "SolidSyslog.h"
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
    nullptr, nullptr, TEST_PATH_PREFIX, TEST_MAX_FILE_SIZE, TEST_MAX_FILES, SOLIDSYSLOG_DISCARD_OLDEST, nullptr,
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
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create(&storage);
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

TEST(SolidSyslogFileStore, HasUnsentFalseAfterAllSent)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
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
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        file = FileFake_Create(&storage);
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
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create(&storage);
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
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create(&storage);
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
    /* MAX_PATH_SIZE=128. A 127-char prefix leaves 1 byte for digits and
       suffix. FormatFilename must not write past the buffer — prior to
       the fix, SolidSyslogFormat_Character wrote 2 bytes unconditionally
       (char + null), overflowing filename[128]. ASan detects this. */
    char prefix[128];
    memset(prefix, 'B', 127);
    prefix[127] = '\0';

    CreateWithPathPrefix(prefix);
    VerifyWriteAndReadBack();
}

TEST(SolidSyslogFileStoreConfig, NullSecurityPolicyDefaultsToNoOp)
{
    struct SolidSyslogFileStoreConfig config = MakeConfig(file);
    config.securityPolicy                    = nullptr;
    store                                    = SolidSyslogFileStore_Create(&config);
    VerifyWriteAndReadBack();
}

/* ------------------------------------------------------------------
 * Error paths
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreErrors)
{
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        file = FileFake_Create(&storage);
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

/* ------------------------------------------------------------------
 * File rotation
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreRotation)
{
    static const size_t RECORD_OVERHEAD    = 5; /* 4 (length) + 1 (sent flag) */
    static const size_t ONE_MAX_MSG_RECORD = SOLIDSYSLOG_MAX_MESSAGE_SIZE + RECORD_OVERHEAD;

    struct FileFakeStorage readStorage = {};
    struct FileFakeStorage writeStorage = {};
    struct SolidSyslogFile* readFile = nullptr;
    struct SolidSyslogFile* writeFile = nullptr;
    struct SolidSyslogStore* store = nullptr;
    char maxMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};

    void setup() override
    {
        readFile = FileFake_Create(&readStorage);
        writeFile = FileFake_Create(&writeStorage);
        memset(maxMsg, 'A', sizeof(maxMsg));
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }

    void CreateWithMaxFileSize(size_t maxFileSize, enum SolidSyslogDiscardPolicy policy = SOLIDSYSLOG_DISCARD_OLDEST,
                               size_t maxFiles = 2)
    {
        struct SolidSyslogFileStoreConfig config = DEFAULT_CONFIG;
        config.readFile      = readFile;
        config.writeFile     = writeFile;
        config.maxFileSize   = maxFileSize;
        config.maxFiles      = maxFiles;
        config.discardPolicy = policy;
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(&config);
    }

    void WriteMaxMsg()
    {
        SolidSyslogStore_Write(store, maxMsg, sizeof(maxMsg));
    }
};

// clang-format on

TEST(SolidSyslogFileStoreRotation, WriteRotatesToNewFileWhenFull)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg();
    WriteMaxMsg();
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store01.log"));
}

TEST(SolidSyslogFileStoreRotation, WriteDoesNotRotateWhenFileHasSpace)
{
    CreateWithMaxFileSize(2 * ONE_MAX_MSG_RECORD);
    WriteMaxMsg();
    WriteMaxMsg();
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store01.log"));
}

TEST(SolidSyslogFileStoreRotation, HasUnsentReturnsTrueAfterRotation)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg();
    WriteMaxMsg();
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, ReadReturnsFirstFileAfterRotation)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char firstMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(firstMsg, 'B', sizeof(firstMsg));
    SolidSyslogStore_Write(store, firstMsg, sizeof(firstMsg));

    WriteMaxMsg(); /* rotates to file 01 */

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    LONGS_EQUAL(sizeof(firstMsg), bytesRead);
    BYTES_EQUAL('B', buf[0]);
}

TEST(SolidSyslogFileStoreRotation, MarkSentAdvancesReadToSecondFile)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char firstMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(firstMsg, 'B', sizeof(firstMsg));
    SolidSyslogStore_Write(store, firstMsg, sizeof(firstMsg));

    WriteMaxMsg(); /* rotates to file 01 */

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    LONGS_EQUAL(SOLIDSYSLOG_MAX_MESSAGE_SIZE, bytesRead);
    BYTES_EQUAL('A', buf[0]);
}

TEST(SolidSyslogFileStoreRotation, FullDrainAcrossTwoFilesHasUnsentFalse)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg();
    WriteMaxMsg();

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    size_t bytesRead = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, DiscardOldestDeletesOldestFileWhenAtMaxFiles)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* file 00 */
    WriteMaxMsg(); /* file 01 — now at maxFiles=2 */
    WriteMaxMsg(); /* file 02 — must discard 00 */

    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store02.log"));
}

TEST(SolidSyslogFileStoreRotation, DiscardOldestSurvivingDataIsReadable)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char firstMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(firstMsg, 'B', sizeof(firstMsg));
    SolidSyslogStore_Write(store, firstMsg, sizeof(firstMsg)); /* file 00 */

    char secondMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(secondMsg, 'C', sizeof(secondMsg));
    SolidSyslogStore_Write(store, secondMsg, sizeof(secondMsg)); /* file 01 */

    WriteMaxMsg(); /* file 02 — discards 00 */

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    LONGS_EQUAL(SOLIDSYSLOG_MAX_MESSAGE_SIZE, bytesRead);
    BYTES_EQUAL('C', buf[0]);
}

TEST(SolidSyslogFileStoreRotation, DiscardNewestReturnsFalseWhenAtMaxFiles)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD, SOLIDSYSLOG_DISCARD_NEWEST);
    WriteMaxMsg(); /* file 00 */
    WriteMaxMsg(); /* file 01 — now at maxFiles=2 */

    CHECK_FALSE(SolidSyslogStore_Write(store, maxMsg, sizeof(maxMsg)));
}

TEST(SolidSyslogFileStoreRotation, ResumeHasUnsentWhenMultipleFilesExist)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* file 00 */
    WriteMaxMsg(); /* file 01 */
    SolidSyslogFileStore_Destroy();

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, ResumeDrainsAcrossFilesInOrder)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char firstMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(firstMsg, 'B', sizeof(firstMsg));
    SolidSyslogStore_Write(store, firstMsg, sizeof(firstMsg)); /* file 00 */

    WriteMaxMsg(); /* file 01 — 'A' */
    SolidSyslogFileStore_Destroy();

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('B', buf[0]);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('A', buf[0]);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, ResumeContinuesWritingToCorrectFile)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* file 00 */
    SolidSyslogFileStore_Destroy();

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* should rotate to file 01, not overwrite 00 */

    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store01.log"));
}

TEST(SolidSyslogFileStoreRotation, ResumeWithMultipleFilesCanWriteNewMessage)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* file 00 */
    WriteMaxMsg(); /* file 01 */
    SolidSyslogFileStore_Destroy();

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char newMsg[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(newMsg, 'N', sizeof(newMsg));
    CHECK_TRUE(SolidSyslogStore_Write(store, newMsg, sizeof(newMsg)));

    /* Should have rotated to file 02 — file 01 was full */
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store02.log"));
}

TEST(SolidSyslogFileStoreRotation, ResumeWriteAppendsToPartiallyFilledWriteFile)
{
    static const size_t TWO_MAX_MSG_RECORDS = 2 * ONE_MAX_MSG_RECORD;

    CreateWithMaxFileSize(TWO_MAX_MSG_RECORDS);
    WriteMaxMsg(); /* file 00, record 1 */
    WriteMaxMsg(); /* file 00, record 2 — file 00 full */
    WriteMaxMsg(); /* file 01, record 1 — file 01 partially filled */
    SolidSyslogFileStore_Destroy();

    CreateWithMaxFileSize(TWO_MAX_MSG_RECORDS);

    /* Write should append to file 01, not rotate */
    CHECK_TRUE(SolidSyslogStore_Write(store, maxMsg, sizeof(maxMsg)));
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store02.log"));
}

TEST(SolidSyslogFileStoreRotation, SequenceWrapsFrom99To00)
{
    /* Pre-seed file 99 so the scan finds it as the write file */
    SolidSyslogFile_Open(writeFile, "/tmp/test_store99.log");
    SolidSyslogFile_Close(writeFile);

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* fills file 99 */
    WriteMaxMsg(); /* should wrap to file 00 */

    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
}

TEST(SolidSyslogFileStoreRotation, WriteAfterDrainRotatesToNextFile)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg();

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));

    /* Drained file still occupies space — next write rotates */
    WriteMaxMsg();
    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store01.log"));
}

TEST(SolidSyslogFileStoreRotation, DestroyClosesBothHandles)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);
    WriteMaxMsg(); /* file 00 */
    WriteMaxMsg(); /* file 01 — read on 00, write on 01 */
    SolidSyslogFileStore_Destroy();

    CHECK_FALSE(SolidSyslogFile_IsOpen(readFile));
    CHECK_FALSE(SolidSyslogFile_IsOpen(writeFile));
}

TEST(SolidSyslogFileStoreRotation, MixedMessageSizesDrainCorrectlyAcrossFiles)
{
    static const size_t SHORT_LEN = 7;

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    char shortMsg[SHORT_LEN];
    memset(shortMsg, 'S', SHORT_LEN);
    SolidSyslogStore_Write(store, shortMsg, SHORT_LEN); /* file 00 — small record */

    WriteMaxMsg(); /* file 01 — max record */

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    LONGS_EQUAL(SHORT_LEN, bytesRead);
    BYTES_EQUAL('S', buf[0]);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    LONGS_EQUAL(SOLIDSYSLOG_MAX_MESSAGE_SIZE, bytesRead);
    BYTES_EQUAL('A', buf[0]);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, ContinuousDiscardWithoutReadingSurvivorsCorrect)
{
    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD);

    /* Write 5 messages across 5 files — maxFiles=2 means 3 are discarded */
    char msgs[5][SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    for (int i = 0; i < 5; i++)
    {
        memset(msgs[i], 'A' + i, sizeof(msgs[i]));               // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        SolidSyslogStore_Write(store, msgs[i], sizeof(msgs[i])); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    /* Only files 03 and 04 should survive */
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store01.log"));
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store02.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store03.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store04.log"));

    /* Drain — should get msg3 ('D') then msg4 ('E') */
    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('D', buf[0]);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('E', buf[0]);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreRotation, MaxFilesAtUpperLimit)
{
    enum
    {
        MAX_FILES = 99
    };

    CreateWithMaxFileSize(ONE_MAX_MSG_RECORD, SOLIDSYSLOG_DISCARD_OLDEST, MAX_FILES);

    /* Fill all 99 files */
    for (int i = 0; i < MAX_FILES; i++)
    {
        WriteMaxMsg();
    }

    /* All 99 files should exist (00–98) */
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store98.log"));
    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store99.log"));

    /* One more write — should discard file 00 and create file 99 */
    WriteMaxMsg();

    CHECK_FALSE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store00.log"));
    CHECK_TRUE(SolidSyslogFile_Exists(writeFile, "/tmp/test_store99.log"));
}

TEST(SolidSyslogFileStoreRotation, MultipleRecordsPerFileDrainAcrossRotation)
{
    static const size_t TWO_MAX_MSG_RECORDS = 2 * ONE_MAX_MSG_RECORD;

    CreateWithMaxFileSize(TWO_MAX_MSG_RECORDS);

    char msg0[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(msg0, 'X', sizeof(msg0));
    SolidSyslogStore_Write(store, msg0, sizeof(msg0)); /* file 00, record 1 */

    char msg1[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    memset(msg1, 'Y', sizeof(msg1));
    SolidSyslogStore_Write(store, msg1, sizeof(msg1)); /* file 00, record 2 */

    WriteMaxMsg(); /* file 01, record 1 — 'A' */

    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE] = {};
    size_t bytesRead                         = 0;

    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('X', buf[0]);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('Y', buf[0]);
    SolidSyslogStore_MarkSent(store);

    memset(buf, 0, sizeof(buf));
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    BYTES_EQUAL('A', buf[0]);
    SolidSyslogStore_MarkSent(store);

    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

/* ------------------------------------------------------------------
 * Integrity (SecurityPolicy integration)
 * ----------------------------------------------------------------*/

static bool     computeIntegrityCalled;
static uint8_t  computeIntegrityData[TEST_BUF_SIZE];
static uint16_t computeIntegrityLength;

// NOLINTNEXTLINE(readability-non-const-parameter) -- matches SecurityPolicy vtable signature
static void SpyComputeIntegrity(const uint8_t* data, uint16_t length, uint8_t* integrityOut)
{
    (void) integrityOut;
    computeIntegrityCalled = true;
    computeIntegrityLength = length;
    memcpy(computeIntegrityData, data, length);
}

static bool     verifyIntegrityCalled;
static uint8_t  verifyIntegrityData[TEST_BUF_SIZE];
static uint16_t verifyIntegrityLength;

static bool SpyVerifyIntegrity(const uint8_t* data, uint16_t length, const uint8_t* integrityIn)
{
    (void) integrityIn;
    verifyIntegrityCalled = true;
    verifyIntegrityLength = length;
    memcpy(verifyIntegrityData, data, length);
    return true;
}

static struct SolidSyslogSecurityPolicy spyPolicy = {
    SpyComputeIntegrity,
    SpyVerifyIntegrity,
};

// clang-format off
TEST_GROUP(SolidSyslogFileStoreIntegrity)
{
    struct FileFakeStorage storage = {};
    struct SolidSyslogFile* file = nullptr;
    struct SolidSyslogStore* store = nullptr;

    void setup() override
    {
        file = FileFake_Create(&storage);
        computeIntegrityCalled  = false;
        computeIntegrityLength  = 0;
        memset(computeIntegrityData, 0, sizeof(computeIntegrityData));
        verifyIntegrityCalled   = false;
        verifyIntegrityLength   = 0;
        memset(verifyIntegrityData, 0, sizeof(verifyIntegrityData));

        struct SolidSyslogFileStoreConfig config = DEFAULT_CONFIG;
        config.readFile       = file;
        config.writeFile      = file;
        config.securityPolicy = &spyPolicy;
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

TEST(SolidSyslogFileStoreIntegrity, WriteCallsComputeIntegrity)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    CHECK_TRUE(computeIntegrityCalled);
}

TEST(SolidSyslogFileStoreIntegrity, ComputeIntegrityReceivesWrittenData)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    LONGS_EQUAL(TEST_DATA_LEN, computeIntegrityLength);
    MEMCMP_EQUAL(TEST_DATA, computeIntegrityData, TEST_DATA_LEN);
}

TEST(SolidSyslogFileStoreIntegrity, ReadCallsVerifyIntegrity)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    CHECK_TRUE(verifyIntegrityCalled);
}

TEST(SolidSyslogFileStoreIntegrity, VerifyIntegrityReceivesWrittenData)
{
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);
    LONGS_EQUAL(TEST_DATA_LEN, verifyIntegrityLength);
    MEMCMP_EQUAL(TEST_DATA, verifyIntegrityData, TEST_DATA_LEN);
}
