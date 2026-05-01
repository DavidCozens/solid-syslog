#include "SolidSyslogFileStore.h"
#include "RecordStore.h"
#include "SolidSyslog.h"
#include "SolidSyslogFile.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogNullSecurityPolicy.h"
#include "SolidSyslogStoreDefinition.h"

#include <stdint.h>

enum
{
    MIN_MAX_FILES    = 2,
    MAX_MAX_FILES    = 99,
    SEQUENCE_MODULUS = 100,
    MAX_PATH_SIZE    = 128
};

/* vtable — forward-declared because InitialiseVtable references them before their definitions */
static bool Write(struct SolidSyslogStore* self, const void* data, size_t size);
static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead);
static void MarkSent(struct SolidSyslogStore* self);
static bool HasUnsent(struct SolidSyslogStore* self);
static bool IsHalted(struct SolidSyslogStore* self);

/* ------------------------------------------------------------------
 * Instance
 * ----------------------------------------------------------------*/

struct SolidSyslogFileStore
{
    struct SolidSyslogStore       base;
    struct RecordStore            recordStore;
    struct SolidSyslogFile*       readFile;
    struct SolidSyslogFile*       writeFile;
    const char*                   pathPrefix;
    size_t                        maxFileSize;
    size_t                        maxFiles;
    enum SolidSyslogDiscardPolicy discardPolicy;
    SolidSyslogStoreFullCallback  onStoreFull;
    bool                          halted;
    uint8_t                       oldestSequence;
    uint8_t                       readSequence;
    uint8_t                       writeSequence;
    size_t                        readCursor;
    size_t                        writePosition;
    bool                          writeFileCorrupt;
};

static const struct SolidSyslogFileStore DEFAULT_INSTANCE = {0};
static struct SolidSyslogFileStore       instance;

/* ------------------------------------------------------------------
 * Shared helpers
 * ----------------------------------------------------------------*/

static inline bool OpenWriteFile(const char* path)
{
    return SolidSyslogFile_Open(instance.writeFile, path);
}

static inline bool OpenReadFile(const char* path)
{
    return SolidSyslogFile_Open(instance.readFile, path);
}

static inline bool IsWriteFileOpen(void)
{
    return (instance.writeFile != NULL) && SolidSyslogFile_IsOpen(instance.writeFile);
}

static inline bool IsReadFileOpen(void)
{
    return (instance.readFile != NULL) && SolidSyslogFile_IsOpen(instance.readFile);
}

static const char FILE_EXTENSION[] = ".log";

enum
{
    SEQUENCE_DIGITS   = 2,
    FILENAME_SUFFIX   = SEQUENCE_DIGITS + sizeof(FILE_EXTENSION) - 1,
    MAX_PREFIX_LENGTH = MAX_PATH_SIZE - FILENAME_SUFFIX - 1
};

static inline const char* FormatFilename(SolidSyslogFormatterStorage* storage, uint8_t sequence)
{
    struct SolidSyslogFormatter* f = SolidSyslogFormatter_Create(storage, MAX_PATH_SIZE);

    SolidSyslogFormatter_BoundedString(f, instance.pathPrefix, MAX_PREFIX_LENGTH);
    SolidSyslogFormatter_TwoDigit(f, sequence);
    SolidSyslogFormatter_BoundedString(f, FILE_EXTENSION, sizeof(FILE_EXTENSION) - 1);

    return SolidSyslogFormatter_AsFormattedBuffer(f);
}

static inline uint8_t NextSequence(uint8_t current)
{
    return (uint8_t) ((current + 1) % SEQUENCE_MODULUS);
}

static inline size_t FileCount(void)
{
    return (size_t) ((instance.writeSequence - instance.oldestSequence + SEQUENCE_MODULUS) % SEQUENCE_MODULUS) + 1;
}

static inline bool ReadingOlderFile(void)
{
    return instance.readSequence != instance.writeSequence;
}

static inline bool HasUnsentRecords(void)
{
    return ReadingOlderFile() || (instance.readCursor < instance.writePosition);
}

/* Create helpers — forward-declared because they appear below Create for top-down reading */
static inline void   ValidateConfig(const struct SolidSyslogFileStoreConfig* config);
static inline size_t ClampToRange(size_t value, size_t min, size_t max);
static inline void   InitialiseVtable(void);
static void          ScanForExistingFiles(void);
static void          ResumeFromExistingFile(void);
static inline void   MeasureFileSize(void);

/* Write helpers — forward-declared because they appear below Write */
static bool        StoreRecord(const void* data, size_t size);
static inline bool FileIsFull(size_t dataSize);
static inline bool StoreIsFull(void);
static inline void NotifyStoreFull(void);
static void        RotateToNextFile(void);
static void        DiscardOldestFile(void);
static void        DeleteOldestFile(void);
static void        SwitchReadFile(uint8_t newSequence);
static bool        MakeSpaceForRecord(size_t dataSize);
static bool        FlushRecord(const void* data, size_t dataSize);

/* ReadNextUnsent helpers */
static void AdvanceToNextReadFile(void);

/* ------------------------------------------------------------------
 * Create
 * ----------------------------------------------------------------*/

struct SolidSyslogStore* SolidSyslogFileStore_Create(const struct SolidSyslogFileStoreConfig* config)
{
    instance            = DEFAULT_INSTANCE;
    instance.readFile   = config->readFile;
    instance.writeFile  = config->writeFile;
    instance.pathPrefix = config->pathPrefix;
    ValidateConfig(config);
    InitialiseVtable();

    ScanForExistingFiles();

    SolidSyslogFormatterStorage writeNameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
    const char*                 writeName = FormatFilename(writeNameStorage, instance.writeSequence);

    if (OpenWriteFile(writeName))
    {
        SolidSyslogFormatterStorage readNameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
        const char*                 readName = FormatFilename(readNameStorage, instance.readSequence);
        OpenReadFile(readName);
        ResumeFromExistingFile();
    }

    return &instance.base;
}

static inline size_t MinFileSize(void)
{
    return RecordStore_RecordSize(&instance.recordStore, SOLIDSYSLOG_MAX_MESSAGE_SIZE);
}

static inline void ValidateConfig(const struct SolidSyslogFileStoreConfig* config)
{
    struct SolidSyslogSecurityPolicy* securityPolicy = config->securityPolicy;

    if (securityPolicy == NULL)
    {
        securityPolicy = SolidSyslogNullSecurityPolicy_Create();
    }

    if (securityPolicy->integritySize > SOLIDSYSLOG_MAX_INTEGRITY_SIZE)
    {
        securityPolicy = SolidSyslogNullSecurityPolicy_Create();
    }

    RecordStore_Init(&instance.recordStore, securityPolicy);

    instance.maxFiles      = ClampToRange(config->maxFiles, MIN_MAX_FILES, MAX_MAX_FILES);
    instance.maxFileSize   = ClampToRange(config->maxFileSize, MinFileSize(), (size_t) -1);
    instance.discardPolicy = config->discardPolicy;
    instance.onStoreFull   = config->onStoreFull;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- value, min, max have distinct semantics
static inline size_t ClampToRange(size_t value, size_t min, size_t max)
{
    size_t result = value;

    if (result < min)
    {
        result = min;
    }

    if (result > max)
    {
        result = max;
    }

    return result;
}

static inline void InitialiseVtable(void)
{
    instance.base.Write          = Write;
    instance.base.ReadNextUnsent = ReadNextUnsent;
    instance.base.MarkSent       = MarkSent;
    instance.base.HasUnsent      = HasUnsent;
    instance.base.IsHalted       = IsHalted;
}

static void ScanForExistingFiles(void)
{
    enum
    {
        MAX_SEQUENCE = 100
    };

    bool foundFirst = false;

    for (int seq = 0; seq < MAX_SEQUENCE; seq++)
    {
        SolidSyslogFormatterStorage nameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
        const char*                 name = FormatFilename(nameStorage, (uint8_t) seq);

        if (SolidSyslogFile_Exists(instance.writeFile, name))
        {
            if (!foundFirst)
            {
                instance.oldestSequence = (uint8_t) seq;
                instance.readSequence   = (uint8_t) seq;
                foundFirst              = true;
            }

            instance.writeSequence = (uint8_t) seq;
        }
    }
}

static void ResumeFromExistingFile(void)
{
    MeasureFileSize();

    bool corrupt        = false;
    instance.readCursor = RecordStore_FindFirstUnsent(&instance.recordStore, instance.readFile, instance.writePosition, &corrupt);

    instance.writeFileCorrupt = corrupt;
}

static inline void MeasureFileSize(void)
{
    instance.writePosition = SolidSyslogFile_Size(instance.writeFile);
}

/* ------------------------------------------------------------------
 * Destroy
 * ----------------------------------------------------------------*/

void SolidSyslogFileStore_Destroy(void)
{
    if (IsWriteFileOpen())
    {
        SolidSyslogFile_Close(instance.writeFile);
    }

    if (IsReadFileOpen())
    {
        SolidSyslogFile_Close(instance.readFile);
    }

    instance = DEFAULT_INSTANCE;
}

/* ------------------------------------------------------------------
 * Write
 * ----------------------------------------------------------------*/

static bool Write(struct SolidSyslogStore* self, const void* data, size_t size)
{
    (void) self;
    bool written = false;

    if (IsWriteFileOpen())
    {
        written = StoreRecord(data, size);
    }

    return written;
}

static bool StoreRecord(const void* data, size_t size)
{
    bool written = false;

    if (MakeSpaceForRecord(size))
    {
        written = FlushRecord(data, size);
    }

    return written;
}

static bool MakeSpaceForRecord(size_t dataSize)
{
    bool spaceAvailable = true;

    if (FileIsFull(dataSize) && StoreIsFull())
    {
        NotifyStoreFull();
        spaceAvailable = false;
    }
    else if (FileIsFull(dataSize))
    {
        RotateToNextFile();
    }

    return spaceAvailable;
}

static inline bool FileIsFull(size_t dataSize)
{
    return instance.writeFileCorrupt || (instance.writePosition + RecordStore_RecordSize(&instance.recordStore, (uint16_t) dataSize)) > instance.maxFileSize;
}

static inline bool StoreIsFull(void)
{
    return (FileCount() >= instance.maxFiles) && (instance.discardPolicy != SOLIDSYSLOG_DISCARD_OLDEST);
}

static inline void NotifyStoreFull(void)
{
    if (instance.discardPolicy == SOLIDSYSLOG_HALT)
    {
        instance.halted = true;

        if (instance.onStoreFull != NULL)
        {
            instance.onStoreFull();
        }
    }
}

static void RotateToNextFile(void)
{
    SolidSyslogFile_Close(instance.writeFile);
    instance.writeSequence    = NextSequence(instance.writeSequence);
    instance.writePosition    = 0;
    instance.writeFileCorrupt = false;
    SolidSyslogFormatterStorage nameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
    const char*                 name = FormatFilename(nameStorage, instance.writeSequence);
    OpenWriteFile(name);

    if (FileCount() > instance.maxFiles)
    {
        DiscardOldestFile();
    }
}

static void DeleteOldestFile(void)
{
    SolidSyslogFormatterStorage nameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
    const char*                 name = FormatFilename(nameStorage, instance.oldestSequence);
    SolidSyslogFile_Delete(instance.writeFile, name);
    instance.oldestSequence = NextSequence(instance.oldestSequence);
}

static void SwitchReadFile(uint8_t newSequence)
{
    SolidSyslogFile_Close(instance.readFile);
    instance.readSequence = newSequence;
    instance.readCursor   = 0;
    RecordStore_ForgetLastRead(&instance.recordStore);
    SolidSyslogFormatterStorage nameStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(MAX_PATH_SIZE)];
    const char*                 name = FormatFilename(nameStorage, instance.readSequence);
    OpenReadFile(name);
}

static void ResetReadToOldestFile(void)
{
    SwitchReadFile(instance.oldestSequence);
}

static void DiscardOldestFile(void)
{
    bool readingOldestFile = (instance.readSequence == instance.oldestSequence);

    DeleteOldestFile();

    if (readingOldestFile)
    {
        ResetReadToOldestFile();
    }
}

static bool FlushRecord(const void* data, size_t dataSize)
{
    bool written = RecordStore_Append(&instance.recordStore, instance.writeFile, instance.writePosition, data, dataSize);

    if (written)
    {
        instance.writePosition += RecordStore_RecordSize(&instance.recordStore, (uint16_t) dataSize);
    }

    return written;
}

/* ------------------------------------------------------------------
 * HasUnsent
 * ----------------------------------------------------------------*/

static bool HasUnsent(struct SolidSyslogStore* self)
{
    (void) self;
    return HasUnsentRecords();
}

static bool IsHalted(struct SolidSyslogStore* self)
{
    (void) self;
    return instance.halted;
}

/* ------------------------------------------------------------------
 * ReadNextUnsent
 * ----------------------------------------------------------------*/

static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead)
{
    (void) self;
    bool read  = false;
    *bytesRead = 0;

    if (HasUnsentRecords())
    {
        read = RecordStore_Read(&instance.recordStore, instance.readFile, instance.readCursor, data, maxSize, bytesRead);

        while (!read && ReadingOlderFile())
        {
            AdvanceToNextReadFile();
            read = RecordStore_Read(&instance.recordStore, instance.readFile, instance.readCursor, data, maxSize, bytesRead);
        }
    }

    return read;
}

static void AdvanceToNextReadFile(void)
{
    SwitchReadFile(NextSequence(instance.readSequence));
}

/* ------------------------------------------------------------------
 * MarkSent
 * ----------------------------------------------------------------*/

static void MarkSent(struct SolidSyslogStore* self)
{
    (void) self;

    size_t nextCursor = 0;

    if (RecordStore_MarkLastReadAsSent(&instance.recordStore, instance.readFile, &nextCursor))
    {
        instance.readCursor = nextCursor;
    }
}
