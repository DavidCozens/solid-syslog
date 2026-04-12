#include "SolidSyslogFileStore.h"
#include "SolidSyslog.h"
#include "SolidSyslogFile.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogStoreDefinition.h"

#include <stdint.h>

enum
{
    RECORD_LENGTH_SIZE = 4,
    SENT_FLAG_SIZE     = 1,
    RECORD_OVERHEAD    = RECORD_LENGTH_SIZE + SENT_FLAG_SIZE,
    SENT_FLAG_UNSENT   = 0,
    SENT_FLAG_SENT     = 1,
    MIN_MAX_FILES      = 2,
    MAX_MAX_FILES      = 99,
    SEQUENCE_MODULUS   = 100,
    MIN_MAX_FILE_SIZE  = SOLIDSYSLOG_MAX_MESSAGE_SIZE + RECORD_OVERHEAD,
    MAX_PATH_SIZE      = 128
};

/* vtable */
static bool Write(struct SolidSyslogStore* self, const void* data, size_t size);
static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead);
static void MarkSent(struct SolidSyslogStore* self);
static bool HasUnsent(struct SolidSyslogStore* self);

/* Create helpers */
static inline void   ValidateConfig(const struct SolidSyslogFileStoreConfig* config);
static inline size_t ClampToRange(size_t value, size_t min, size_t max);
static inline void   FormatFilename(uint8_t sequence);
static void          ScanForExistingFiles(void);
static inline void   InitialiseVtable(void);
static inline bool   OpenWriteFile(const char* path);
static inline bool   OpenReadFile(const char* path);
static inline bool   IsWriteFileOpen(void);
static inline bool   IsReadFileOpen(void);
static void          ResumeFromExistingFile(void);
static inline void   MeasureFileSize(void);
static inline void   FindFirstUnsentRecord(void);
static size_t        ScanForFirstUnsent(size_t fileSize);
static bool          AdvancePastSentRecord(size_t* cursor, size_t fileSize);
static bool          ReadRecordLength(size_t offset, uint32_t* length);
static inline bool   ReadExact(void* buf, size_t count);
static inline bool   IsRecordSent(size_t recordStart, uint32_t length);
static bool          ReadSentFlag(size_t recordStart, uint32_t dataLength, uint8_t* flag);
static inline size_t SentFlagOffset(size_t recordStart, uint32_t dataLength);
static inline void   SkipRecord(size_t* cursor, uint32_t length);
static inline size_t RecordSize(uint32_t dataLength);
static inline void   SkipToEndOfValidData(size_t* cursor, size_t fileSize);

/* File rotation helpers */
static inline bool    StoreIsFull(void);
static inline bool    FileIsFull(size_t dataSize);
static inline size_t  FileCount(void);
static inline bool    ReadingOlderFile(void);
static inline uint8_t NextSequence(uint8_t current);
static void           RotateToNextFile(void);
static void           DiscardOldestFile(void);

/* Write helpers */
static bool        WriteRecordToFile(const void* data, size_t size);
static inline void SeekToWritePosition(void);
static inline bool WriteRecordHeader(size_t dataSize);
static inline bool WriteExact(const void* buf, size_t count);
static inline bool WriteRecordBody(const void* data, size_t size);
static inline bool WriteUnsentFlag(void);
static inline void AdvanceWritePosition(size_t dataSize);

/* HasUnsent helpers */
static inline bool HasUnsentRecords(void);

/* ReadNextUnsent helpers */
static bool          ReadCurrentRecord(void* data, size_t maxSize, size_t* bytesRead);
static inline void   RememberCurrentRecord(uint32_t length);
static bool          ReadRecordData(size_t recordStart, uint32_t length, void* data, size_t maxSize, size_t* bytesRead);
static inline size_t DataOffset(size_t recordStart);
static inline size_t BoundedSize(uint32_t length, size_t maxSize);

/* MarkSent helpers */
static inline bool WriteSentFlag(void);
static inline void AdvanceReadCursor(void);

struct SolidSyslogFileStore
{
    struct SolidSyslogStore       base;
    struct SolidSyslogFile*       readFile;
    struct SolidSyslogFile*       writeFile;
    const char*                   pathPrefix;
    char                          filename[MAX_PATH_SIZE];
    size_t                        maxFileSize;
    size_t                        maxFiles;
    enum SolidSyslogDiscardPolicy discardPolicy;
    uint8_t                       oldestSequence;
    uint8_t                       readSequence;
    uint8_t                       writeSequence;
    size_t                        readCursor;
    size_t                        writePosition;
    size_t                        lastSentFlagOffset;
    bool                          hasReadRecord;
};

static const struct SolidSyslogFileStore DEFAULT_INSTANCE = {0};
static struct SolidSyslogFileStore       instance;

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

    FormatFilename(instance.writeSequence);

    if (OpenWriteFile(instance.filename))
    {
        FormatFilename(instance.readSequence);
        OpenReadFile(instance.filename);
        ResumeFromExistingFile();
    }

    return &instance.base;
}

static inline void ValidateConfig(const struct SolidSyslogFileStoreConfig* config)
{
    instance.maxFiles      = ClampToRange(config->maxFiles, MIN_MAX_FILES, MAX_MAX_FILES);
    instance.maxFileSize   = ClampToRange(config->maxFileSize, MIN_MAX_FILE_SIZE, (size_t) -1);
    instance.discardPolicy = config->discardPolicy;
}

static inline void FormatFilename(uint8_t sequence)
{
    enum
    {
        TENS_DIVISOR = 10
    };

    size_t len = 0;
    len += SolidSyslogFormat_BoundedString(instance.filename + len, instance.pathPrefix, MAX_PATH_SIZE - len);

    if ((len + 1) < MAX_PATH_SIZE)
    {
        len += SolidSyslogFormat_Character(instance.filename + len, SolidSyslogFormat_DigitToChar(sequence / TENS_DIVISOR));
    }

    if ((len + 1) < MAX_PATH_SIZE)
    {
        len += SolidSyslogFormat_Character(instance.filename + len, SolidSyslogFormat_DigitToChar(sequence % TENS_DIVISOR));
    }

    len += SolidSyslogFormat_BoundedString(instance.filename + len, ".log", MAX_PATH_SIZE - len);
    instance.filename[len] = '\0';
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

static void ScanForExistingFiles(void)
{
    enum
    {
        MAX_SEQUENCE = 100
    };

    bool foundFirst = false;

    for (int seq = 0; seq < MAX_SEQUENCE; seq++)
    {
        FormatFilename((uint8_t) seq);

        if (SolidSyslogFile_Exists(instance.writeFile, instance.filename))
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

static inline void InitialiseVtable(void)
{
    instance.base.Write          = Write;
    instance.base.ReadNextUnsent = ReadNextUnsent;
    instance.base.MarkSent       = MarkSent;
    instance.base.HasUnsent      = HasUnsent;
}

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

static void ResumeFromExistingFile(void)
{
    MeasureFileSize();
    FindFirstUnsentRecord();
}

static inline void MeasureFileSize(void)
{
    instance.writePosition = SolidSyslogFile_Size(instance.writeFile);
}

static inline void FindFirstUnsentRecord(void)
{
    instance.readCursor = ScanForFirstUnsent(instance.writePosition);
}

static size_t ScanForFirstUnsent(size_t fileSize)
{
    size_t cursor   = 0;
    bool   scanning = true;

    while (scanning && (cursor < fileSize))
    {
        scanning = AdvancePastSentRecord(&cursor, fileSize);
    }

    return cursor;
}

static bool AdvancePastSentRecord(size_t* cursor, size_t fileSize)
{
    uint32_t length   = 0;
    bool     advanced = false;

    if (ReadRecordLength(*cursor, &length))
    {
        if (IsRecordSent(*cursor, length))
        {
            SkipRecord(cursor, length);
            advanced = true;
        }
    }
    else
    {
        SkipToEndOfValidData(cursor, fileSize);
    }

    return advanced;
}

static bool ReadRecordLength(size_t offset, uint32_t* length)
{
    SolidSyslogFile_SeekTo(instance.readFile, offset);
    return ReadExact(length, RECORD_LENGTH_SIZE);
}

static inline bool ReadExact(void* buf, size_t count)
{
    return SolidSyslogFile_Read(instance.readFile, buf, count);
}

static inline bool IsRecordSent(size_t recordStart, uint32_t length)
{
    uint8_t flag = SENT_FLAG_SENT;
    ReadSentFlag(recordStart, length, &flag);
    return flag == SENT_FLAG_SENT;
}

static bool ReadSentFlag(size_t recordStart, uint32_t dataLength, uint8_t* flag)
{
    SolidSyslogFile_SeekTo(instance.readFile, SentFlagOffset(recordStart, dataLength));
    return ReadExact(flag, SENT_FLAG_SIZE);
}

static inline size_t SentFlagOffset(size_t recordStart, uint32_t dataLength)
{
    return recordStart + RECORD_LENGTH_SIZE + dataLength;
}

static inline void SkipRecord(size_t* cursor, uint32_t length)
{
    *cursor += RecordSize(length);
}

static inline size_t RecordSize(uint32_t dataLength)
{
    return RECORD_LENGTH_SIZE + dataLength + SENT_FLAG_SIZE;
}

static inline void SkipToEndOfValidData(size_t* cursor, size_t fileSize)
{
    *cursor = fileSize;
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
        written = WriteRecordToFile(data, size);
    }

    return written;
}

static bool WriteRecordToFile(const void* data, size_t size)
{
    bool written = false;

    if (FileIsFull(size))
    {
        if (StoreIsFull())
        {
            return false;
        }

        RotateToNextFile();
    }

    SeekToWritePosition();

    if (WriteRecordHeader(size) && WriteRecordBody(data, size) && WriteUnsentFlag())
    {
        AdvanceWritePosition(size);
        written = true;
    }

    return written;
}

static inline bool StoreIsFull(void)
{
    return (FileCount() >= instance.maxFiles) && (instance.discardPolicy == SOLIDSYSLOG_DISCARD_NEWEST);
}

static inline bool FileIsFull(size_t dataSize)
{
    return (instance.writePosition + RecordSize((uint32_t) dataSize)) > instance.maxFileSize;
}

static void RotateToNextFile(void)
{
    SolidSyslogFile_Close(instance.writeFile);
    instance.writeSequence = NextSequence(instance.writeSequence);
    instance.writePosition = 0;
    FormatFilename(instance.writeSequence);
    OpenWriteFile(instance.filename);

    if (FileCount() > instance.maxFiles)
    {
        DiscardOldestFile();
    }
}

static inline uint8_t NextSequence(uint8_t current)
{
    return (uint8_t) ((current + 1) % SEQUENCE_MODULUS);
}

static inline size_t FileCount(void)
{
    return (size_t) ((instance.writeSequence - instance.oldestSequence + SEQUENCE_MODULUS) % SEQUENCE_MODULUS) + 1;
}

static void DiscardOldestFile(void)
{
    FormatFilename(instance.oldestSequence);
    SolidSyslogFile_Delete(instance.writeFile, instance.filename);
    instance.oldestSequence = NextSequence(instance.oldestSequence);
}

static inline void SeekToWritePosition(void)
{
    SolidSyslogFile_SeekTo(instance.writeFile, instance.writePosition);
}

static inline bool WriteRecordHeader(size_t dataSize)
{
    uint32_t length = (uint32_t) dataSize;
    return WriteExact(&length, RECORD_LENGTH_SIZE);
}

static inline bool WriteExact(const void* buf, size_t count)
{
    return SolidSyslogFile_Write(instance.writeFile, buf, count);
}

static inline bool WriteRecordBody(const void* data, size_t size)
{
    return WriteExact(data, size);
}

static inline bool WriteUnsentFlag(void)
{
    uint8_t flag = SENT_FLAG_UNSENT;
    return WriteExact(&flag, SENT_FLAG_SIZE);
}

static inline void AdvanceWritePosition(size_t dataSize)
{
    instance.writePosition += RecordSize((uint32_t) dataSize);
}

/* ------------------------------------------------------------------
 * HasUnsent
 * ----------------------------------------------------------------*/

static bool HasUnsent(struct SolidSyslogStore* self)
{
    (void) self;
    return HasUnsentRecords();
}

static inline bool ReadingOlderFile(void)
{
    return instance.readSequence != instance.writeSequence;
}

static inline bool HasUnsentRecords(void)
{
    return ReadingOlderFile() || (instance.readCursor < instance.writePosition);
}

/* ------------------------------------------------------------------
 * ReadNextUnsent
 * ----------------------------------------------------------------*/

static void AdvanceToNextReadFile(void);

static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead)
{
    (void) self;
    bool read  = false;
    *bytesRead = 0;

    if (HasUnsentRecords())
    {
        read = ReadCurrentRecord(data, maxSize, bytesRead);

        while (!read && ReadingOlderFile())
        {
            AdvanceToNextReadFile();
            read = ReadCurrentRecord(data, maxSize, bytesRead);
        }
    }

    return read;
}

static void AdvanceToNextReadFile(void)
{
    SolidSyslogFile_Close(instance.readFile);
    instance.readSequence = NextSequence(instance.readSequence);
    instance.readCursor   = 0;
    FormatFilename(instance.readSequence);
    OpenReadFile(instance.filename);
}

static bool ReadCurrentRecord(void* data, size_t maxSize, size_t* bytesRead)
{
    uint32_t length = 0;
    bool     read   = false;

    if (ReadRecordLength(instance.readCursor, &length) && ReadRecordData(instance.readCursor, length, data, maxSize, bytesRead))
    {
        RememberCurrentRecord(length);
        read = true;
    }

    return read;
}

static inline void RememberCurrentRecord(uint32_t length)
{
    instance.lastSentFlagOffset = SentFlagOffset(instance.readCursor, length);
    instance.hasReadRecord      = true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- recordStart is a file offset, length is a data size; distinct semantics
static bool ReadRecordData(size_t recordStart, uint32_t length, void* data, size_t maxSize, size_t* bytesRead)
{
    size_t copySize = BoundedSize(length, maxSize);

    SolidSyslogFile_SeekTo(instance.readFile, DataOffset(recordStart));

    if (ReadExact(data, copySize))
    {
        *bytesRead = copySize;
    }

    return *bytesRead > 0;
}

static inline size_t DataOffset(size_t recordStart)
{
    return recordStart + RECORD_LENGTH_SIZE;
}

static inline size_t BoundedSize(uint32_t length, size_t maxSize)
{
    return (length < maxSize) ? length : maxSize;
}

/* ------------------------------------------------------------------
 * MarkSent
 * ----------------------------------------------------------------*/

static void MarkSent(struct SolidSyslogStore* self)
{
    (void) self;

    if (instance.hasReadRecord && WriteSentFlag())
    {
        AdvanceReadCursor();
    }
}

static inline bool WriteSentFlag(void)
{
    uint8_t flag = SENT_FLAG_SENT;

    SolidSyslogFile_SeekTo(instance.readFile, instance.lastSentFlagOffset);
    return SolidSyslogFile_Write(instance.readFile, &flag, SENT_FLAG_SIZE);
}

static inline void AdvanceReadCursor(void)
{
    instance.readCursor    = instance.lastSentFlagOffset + SENT_FLAG_SIZE;
    instance.hasReadRecord = false;
}
