#include "CppUTest/TestHarness.h"
#include "SolidSyslogFileStore.h"
#include "FileFake.h"

#include <cstring>

static const char* const TEST_PATH     = "/tmp/test_store.dat";
static const char* const TEST_DATA     = "hello";
static const size_t      TEST_DATA_LEN = 5;

enum
{
    TEST_BUF_SIZE = 512,
    SENTINEL      = 'Z'
};

/* ------------------------------------------------------------------
 * Basic operations
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStore)
{
    struct SolidSyslogFileApi* fileApi = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        fileApi = FileFake_Create();
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
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
    struct SolidSyslogFileApi* fileApi = nullptr;
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        fileApi = FileFake_Create();
    }

    void teardown() override
    {
        SolidSyslogFileStore_Destroy();
        FileFake_Destroy();
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- total and markedSent have distinct semantics
    void WritePreviousSession(int total, int markedSent)
    {
        // cppcheck-suppress unreadVariable -- used by WriteMessages/DrainMessages; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
        WriteMessages(total);
        DrainMessages(markedSent);
        SolidSyslogFileStore_Destroy();

        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
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
    struct SolidSyslogFileApi* fileApi = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        fileApi = FileFake_Create();
    }

    void teardown() override
    {
        FileFake_Destroy();
    }
};

// clang-format on

TEST(SolidSyslogFileStoreDestroy, DestroyClosesFile)
{
    SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    CHECK_TRUE(SolidSyslogFileApi_IsOpen(fileApi));
    SolidSyslogFileStore_Destroy();
    CHECK_FALSE(SolidSyslogFileApi_IsOpen(fileApi));
}

TEST(SolidSyslogFileStoreDestroy, DoubleDestroyDoesNotCrash)
{
    SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    SolidSyslogFileStore_Destroy();
    SolidSyslogFileStore_Destroy();
}

/* ------------------------------------------------------------------
 * Error paths
 * ----------------------------------------------------------------*/

// clang-format off
TEST_GROUP(SolidSyslogFileStoreErrors)
{
    struct SolidSyslogFileApi* fileApi = nullptr;
    // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
    struct SolidSyslogStore*   store   = nullptr;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- used across TEST_GROUP methods; cppcheck does not model CppUTest macros
        fileApi = FileFake_Create();
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
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    CHECK_TRUE(store != nullptr);
}

TEST(SolidSyslogFileStoreErrors, WriteReturnsFalseWhenNotOpen)
{
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    CHECK_FALSE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
}

TEST(SolidSyslogFileStoreErrors, WriteReturnsFalseOnWriteFailure)
{
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    FileFake_FailNextWrite();
    CHECK_FALSE(SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN));
}

TEST(SolidSyslogFileStoreErrors, ReadReturnsFalseOnReadFailure)
{
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);
    FileFake_FailNextRead();

    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    CHECK_FALSE(SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead));
    LONGS_EQUAL(0, bytesRead);
}

TEST(SolidSyslogFileStoreErrors, HasUnsentReturnsFalseWhenNotOpen)
{
    FileFake_FailNextOpen();
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    CHECK_FALSE(SolidSyslogStore_HasUnsent(store));
}

TEST(SolidSyslogFileStoreErrors, MarkSentDoesNotAdvanceWhenWriteFails)
{
    store = SolidSyslogFileStore_Create(fileApi, TEST_PATH);
    SolidSyslogStore_Write(store, TEST_DATA, TEST_DATA_LEN);

    char   buf[TEST_BUF_SIZE];
    size_t bytesRead = 0;
    SolidSyslogStore_ReadNextUnsent(store, buf, sizeof(buf), &bytesRead);

    FileFake_FailNextWrite();
    SolidSyslogStore_MarkSent(store);

    CHECK_TRUE(SolidSyslogStore_HasUnsent(store));
}
