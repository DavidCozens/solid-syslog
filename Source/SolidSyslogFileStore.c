#include "SolidSyslogFileStore.h"
#include "SolidSyslogFileApi.h"
#include "SolidSyslogStoreDefinition.h"

#include <stdint.h>

enum
{
    RECORD_LENGTH_SIZE = 4,
    SENT_FLAG_SIZE     = 1,
    SENT_FLAG_UNSENT   = 0,
    SENT_FLAG_SENT     = 1
};

/* vtable */
static bool Write(struct SolidSyslogStore* self, const void* data, size_t size);
static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead);
static void MarkSent(struct SolidSyslogStore* self);
static bool HasUnsent(struct SolidSyslogStore* self);

/* Create helpers */
static inline void   InitialiseVtable(void);
static inline bool   OpenFile(const char* path);
static inline bool   IsOpen(void);
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
static void          TruncateIfAllSent(void);
static inline bool   AllRecordsSent(void);
static inline void   ResetFile(void);

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
static inline void WriteSentFlag(void);
static inline void AdvanceReadCursor(void);

struct SolidSyslogFileStore
{
    struct SolidSyslogStore    base;
    struct SolidSyslogFileApi* fileApi;
    size_t                     readCursor;
    size_t                     writePosition;
    size_t                     lastSentFlagOffset;
    bool                       hasReadRecord;
};

static const struct SolidSyslogFileStore DEFAULT_INSTANCE = {0};
static struct SolidSyslogFileStore       instance;

/* ------------------------------------------------------------------
 * Create
 * ----------------------------------------------------------------*/

struct SolidSyslogStore* SolidSyslogFileStore_Create(struct SolidSyslogFileApi* fileApi, const char* path)
{
    instance         = DEFAULT_INSTANCE;
    instance.fileApi = fileApi;
    InitialiseVtable();

    if (OpenFile(path))
    {
        ResumeFromExistingFile();
    }

    return &instance.base;
}

static inline void InitialiseVtable(void)
{
    instance.base.Write          = Write;
    instance.base.ReadNextUnsent = ReadNextUnsent;
    instance.base.MarkSent       = MarkSent;
    instance.base.HasUnsent      = HasUnsent;
}

static inline bool OpenFile(const char* path)
{
    return SolidSyslogFileApi_Open(instance.fileApi, path);
}

static inline bool IsOpen(void)
{
    return (instance.fileApi != NULL) && SolidSyslogFileApi_IsOpen(instance.fileApi);
}

static void ResumeFromExistingFile(void)
{
    MeasureFileSize();
    FindFirstUnsentRecord();
    TruncateIfAllSent();
}

static inline void MeasureFileSize(void)
{
    instance.writePosition = SolidSyslogFileApi_Size(instance.fileApi);
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
    SolidSyslogFileApi_SeekTo(instance.fileApi, offset);
    return ReadExact(length, RECORD_LENGTH_SIZE);
}

static inline bool ReadExact(void* buf, size_t count)
{
    return SolidSyslogFileApi_Read(instance.fileApi, buf, count);
}

static inline bool IsRecordSent(size_t recordStart, uint32_t length)
{
    uint8_t flag = SENT_FLAG_SENT;
    ReadSentFlag(recordStart, length, &flag);
    return flag == SENT_FLAG_SENT;
}

static bool ReadSentFlag(size_t recordStart, uint32_t dataLength, uint8_t* flag)
{
    SolidSyslogFileApi_SeekTo(instance.fileApi, SentFlagOffset(recordStart, dataLength));
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

static void TruncateIfAllSent(void)
{
    if (AllRecordsSent())
    {
        ResetFile();
    }
}

static inline bool AllRecordsSent(void)
{
    return (instance.readCursor >= instance.writePosition) && (instance.writePosition > 0);
}

static inline void ResetFile(void)
{
    SolidSyslogFileApi_Truncate(instance.fileApi);
    instance.readCursor    = 0;
    instance.writePosition = 0;
}

/* ------------------------------------------------------------------
 * Destroy
 * ----------------------------------------------------------------*/

void SolidSyslogFileStore_Destroy(void)
{
    if (IsOpen())
    {
        SolidSyslogFileApi_Close(instance.fileApi);
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

    if (IsOpen())
    {
        written = WriteRecordToFile(data, size);
    }

    return written;
}

static bool WriteRecordToFile(const void* data, size_t size)
{
    bool written = false;

    SeekToWritePosition();

    if (WriteRecordHeader(size) && WriteRecordBody(data, size) && WriteUnsentFlag())
    {
        AdvanceWritePosition(size);
        written = true;
    }

    return written;
}

static inline void SeekToWritePosition(void)
{
    SolidSyslogFileApi_SeekTo(instance.fileApi, instance.writePosition);
}

static inline bool WriteRecordHeader(size_t dataSize)
{
    uint32_t length = (uint32_t) dataSize;
    return WriteExact(&length, RECORD_LENGTH_SIZE);
}

static inline bool WriteExact(const void* buf, size_t count)
{
    return SolidSyslogFileApi_Write(instance.fileApi, buf, count);
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

static inline bool HasUnsentRecords(void)
{
    return instance.readCursor < instance.writePosition;
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
        read = ReadCurrentRecord(data, maxSize, bytesRead);
    }

    return read;
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

    SolidSyslogFileApi_SeekTo(instance.fileApi, DataOffset(recordStart));

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

    if (instance.hasReadRecord)
    {
        WriteSentFlag();
        AdvanceReadCursor();
        TruncateIfAllSent();
    }
}

static inline void WriteSentFlag(void)
{
    uint8_t flag = SENT_FLAG_SENT;

    SolidSyslogFileApi_SeekTo(instance.fileApi, instance.lastSentFlagOffset);
    WriteExact(&flag, SENT_FLAG_SIZE);
}

static inline void AdvanceReadCursor(void)
{
    instance.readCursor    = instance.lastSentFlagOffset + SENT_FLAG_SIZE;
    instance.hasReadRecord = false;
}
