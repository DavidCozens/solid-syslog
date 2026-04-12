#include "SolidSyslogFileStore.h"
#include "SolidSyslog.h"
#include "SolidSyslogFile.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogNullSecurityPolicy.h"
#include "SolidSyslogStoreDefinition.h"

#include <stdint.h>
#include <string.h>

enum
{
    MAGIC_SIZE         = 2,
    MAGIC_BYTE_0       = 0xA5,
    MAGIC_BYTE_1       = 0x5A,
    RECORD_LENGTH_SIZE = 2,
    SENT_FLAG_SIZE     = 1,
    RECORD_OVERHEAD    = MAGIC_SIZE + RECORD_LENGTH_SIZE + SENT_FLAG_SIZE,
    SENT_FLAG_UNSENT   = 0xFF,
    SENT_FLAG_SENT     = 0x00,
    MIN_MAX_FILES      = 2,
    MAX_MAX_FILES      = 99,
    SEQUENCE_MODULUS   = 100,
    MIN_MAX_FILE_SIZE  = SOLIDSYSLOG_MAX_MESSAGE_SIZE + RECORD_OVERHEAD,
    MAX_PATH_SIZE      = 128
};

/* vtable — forward-declared because InitialiseVtable references them before their definitions */
static bool Write(struct SolidSyslogStore* self, const void* data, size_t size);
static bool ReadNextUnsent(struct SolidSyslogStore* self, void* data, size_t maxSize, size_t* bytesRead);
static void MarkSent(struct SolidSyslogStore* self);
static bool HasUnsent(struct SolidSyslogStore* self);

/* ------------------------------------------------------------------
 * Record buffer — single static buffer for integrity computation
 * ----------------------------------------------------------------*/

enum
{
    INTEGRITY_BUFFER_SIZE = MAGIC_SIZE + RECORD_LENGTH_SIZE + SOLIDSYSLOG_MAX_MESSAGE_SIZE + SOLIDSYSLOG_MAX_INTEGRITY_SIZE
};

static uint8_t recordBuffer[INTEGRITY_BUFFER_SIZE];

static inline uint8_t* MagicAddress(void)
{
    return recordBuffer;
}

static inline uint8_t* LengthAddress(void)
{
    return recordBuffer + MAGIC_SIZE;
}

static inline uint8_t* MessageAddress(void)
{
    return recordBuffer + MAGIC_SIZE + RECORD_LENGTH_SIZE;
}

static inline uint8_t* IntegrityChecksumAddress(size_t dataSize)
{
    return recordBuffer + MAGIC_SIZE + RECORD_LENGTH_SIZE + dataSize;
}

static inline uint8_t* IntegrityRegionAddress(void)
{
    return MagicAddress();
}

static inline uint16_t IntegrityRegionSize(size_t dataSize)
{
    return (uint16_t) (MAGIC_SIZE + RECORD_LENGTH_SIZE + dataSize);
}

/* ------------------------------------------------------------------
 * Instance
 * ----------------------------------------------------------------*/

struct SolidSyslogFileStore
{
    struct SolidSyslogStore           base;
    struct SolidSyslogFile*           readFile;
    struct SolidSyslogFile*           writeFile;
    struct SolidSyslogSecurityPolicy* securityPolicy;
    const char*                       pathPrefix;
    char                              filename[MAX_PATH_SIZE];
    size_t                            maxFileSize;
    size_t                            maxFiles;
    enum SolidSyslogDiscardPolicy     discardPolicy;
    uint8_t                           oldestSequence;
    uint8_t                           readSequence;
    uint8_t                           writeSequence;
    size_t                            readCursor;
    size_t                            writePosition;
    size_t                            lastSentFlagOffset;
    bool                              hasReadRecord;
};

static const struct SolidSyslogFileStore DEFAULT_INSTANCE = {0};
static struct SolidSyslogFileStore       instance;

/* ------------------------------------------------------------------
 * Shared helpers — used by both Create (resume scan) and ReadNextUnsent
 * ----------------------------------------------------------------*/

static inline bool ReadExact(void* buf, size_t count)
{
    return SolidSyslogFile_Read(instance.readFile, buf, count);
}

static inline bool WriteExact(const void* buf, size_t count)
{
    return SolidSyslogFile_Write(instance.writeFile, buf, count);
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

static inline size_t RecordSize(uint16_t dataLength)
{
    return (size_t) MAGIC_SIZE + RECORD_LENGTH_SIZE + dataLength + instance.securityPolicy->integrity_size + SENT_FLAG_SIZE;
}

static inline size_t SentFlagOffset(size_t recordStart, uint16_t dataLength)
{
    return recordStart + MAGIC_SIZE + RECORD_LENGTH_SIZE + dataLength + instance.securityPolicy->integrity_size;
}

static inline size_t DataOffset(size_t recordStart)
{
    return recordStart + MAGIC_SIZE + RECORD_LENGTH_SIZE;
}

static inline bool ReadMagic(size_t offset)
{
    uint8_t magic[MAGIC_SIZE] = {0};

    SolidSyslogFile_SeekTo(instance.readFile, offset);

    return ReadExact(magic, MAGIC_SIZE) && (magic[0] == MAGIC_BYTE_0) && (magic[1] == MAGIC_BYTE_1);
}

static bool ReadRecordLength(size_t offset, uint16_t* length)
{
    SolidSyslogFile_SeekTo(instance.readFile, offset);
    return ReadExact(length, RECORD_LENGTH_SIZE);
}

static bool ReadSentFlag(size_t recordStart, uint16_t dataLength, uint8_t* flag)
{
    SolidSyslogFile_SeekTo(instance.readFile, SentFlagOffset(recordStart, dataLength));
    return ReadExact(flag, SENT_FLAG_SIZE);
}

static inline bool IsRecordSent(size_t recordStart, uint16_t length)
{
    uint8_t flag = SENT_FLAG_SENT;
    ReadSentFlag(recordStart, length, &flag);
    return flag == SENT_FLAG_SENT;
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
static inline void   FindFirstUnsentRecord(void);
static size_t        ScanForFirstUnsent(size_t fileSize);
static bool          AdvancePastSentRecord(size_t* cursor, size_t fileSize);
static inline void   SkipRecord(size_t* cursor, uint16_t length);
static inline void   SkipToEndOfValidData(size_t* cursor, size_t fileSize);

/* Write helpers — forward-declared because they appear below Write */
static bool        WriteRecordToFile(const void* data, size_t size);
static inline bool FileIsFull(size_t dataSize);
static inline bool StoreIsFull(void);
static void        RotateToNextFile(void);
static void        DiscardOldestFile(void);
static inline void SeekToWritePosition(void);
static inline bool WriteMagic(void);
static inline bool WriteRecordHeader(size_t dataSize);
static inline bool WriteRecordBody(const void* data, size_t size);
static bool        ComputeAndWriteIntegrity(const void* data, size_t size);
static inline void AssembleIntegrityRegion(const void* data, size_t size);
static inline bool WriteIntegrity(size_t dataSize);
static inline bool WriteUnsentFlag(void);
static inline void AdvanceWritePosition(size_t dataSize);

/* ReadNextUnsent helpers */
static void AdvanceToNextReadFile(void);
static bool ReadCurrentRecord(void* data, size_t maxSize, size_t* bytesRead);
static bool VerifyIntegrity(size_t recordStart, uint16_t length);
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- recordStart is a file offset, regionSize is a byte count; distinct semantics
static bool          ReadIntegrityRegion(size_t recordStart, uint16_t regionSize);
static inline bool   ReadIntegrity(size_t recordStart, uint16_t dataLength);
static inline size_t IntegrityOffset(size_t recordStart, uint16_t dataLength);
static bool          ReadRecordData(size_t recordStart, uint16_t length, void* data, size_t maxSize, size_t* bytesRead);
static inline size_t BoundedSize(uint16_t length, size_t maxSize);
static inline void   RememberCurrentRecord(uint16_t length);

/* MarkSent helpers */
static inline bool WriteSentFlag(void);
static inline void AdvanceReadCursor(void);

/* ------------------------------------------------------------------
 * Create
 * ----------------------------------------------------------------*/

struct SolidSyslogStore* SolidSyslogFileStore_Create(const struct SolidSyslogFileStoreConfig* config)
{
    instance                = DEFAULT_INSTANCE;
    instance.readFile       = config->readFile;
    instance.writeFile      = config->writeFile;
    instance.securityPolicy = config->securityPolicy;
    instance.pathPrefix     = config->pathPrefix;
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

    if (instance.securityPolicy == NULL)
    {
        instance.securityPolicy = SolidSyslogNullSecurityPolicy_Create();
    }
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
    uint16_t length   = 0;
    bool     advanced = false;

    if (ReadMagic(*cursor) && ReadRecordLength(*cursor + MAGIC_SIZE, &length))
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

static inline void SkipRecord(size_t* cursor, uint16_t length)
{
    *cursor += RecordSize(length);
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

    if (WriteMagic() && WriteRecordHeader(size) && WriteRecordBody(data, size) && ComputeAndWriteIntegrity(data, size) && WriteUnsentFlag())
    {
        AdvanceWritePosition(size);
        written = true;
    }

    return written;
}

static inline bool FileIsFull(size_t dataSize)
{
    return (instance.writePosition + RecordSize((uint16_t) dataSize)) > instance.maxFileSize;
}

static inline bool StoreIsFull(void)
{
    return (FileCount() >= instance.maxFiles) && (instance.discardPolicy == SOLIDSYSLOG_DISCARD_NEWEST);
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

static inline bool WriteMagic(void)
{
    uint8_t magic[MAGIC_SIZE] = {MAGIC_BYTE_0, MAGIC_BYTE_1};
    return WriteExact(magic, MAGIC_SIZE);
}

static inline bool WriteRecordHeader(size_t dataSize)
{
    uint16_t length = (uint16_t) dataSize;
    return WriteExact(&length, RECORD_LENGTH_SIZE);
}

static inline bool WriteRecordBody(const void* data, size_t size)
{
    return WriteExact(data, size);
}

static bool ComputeAndWriteIntegrity(const void* data, size_t size)
{
    AssembleIntegrityRegion(data, size);
    instance.securityPolicy->ComputeIntegrity(IntegrityRegionAddress(), IntegrityRegionSize(size), IntegrityChecksumAddress(size));

    return WriteIntegrity(size);
}

static inline void AssembleIntegrityRegion(const void* data, size_t size)
{
    MagicAddress()[0] = MAGIC_BYTE_0;
    MagicAddress()[1] = MAGIC_BYTE_1;

    uint16_t length = (uint16_t) size;
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded size
    memcpy(LengthAddress(), &length, RECORD_LENGTH_SIZE);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded size
    memcpy(MessageAddress(), data, size);
}

static inline bool WriteIntegrity(size_t dataSize)
{
    return WriteExact(IntegrityChecksumAddress(dataSize), instance.securityPolicy->integrity_size);
}

static inline bool WriteUnsentFlag(void)
{
    uint8_t flag = SENT_FLAG_UNSENT;
    return WriteExact(&flag, SENT_FLAG_SIZE);
}

static inline void AdvanceWritePosition(size_t dataSize)
{
    instance.writePosition += RecordSize((uint16_t) dataSize);
}

/* ------------------------------------------------------------------
 * HasUnsent
 * ----------------------------------------------------------------*/

static bool HasUnsent(struct SolidSyslogStore* self)
{
    (void) self;
    return HasUnsentRecords();
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
    uint16_t length = 0;
    bool     read   = false;

    if (ReadMagic(instance.readCursor) && ReadRecordLength(instance.readCursor + MAGIC_SIZE, &length) && VerifyIntegrity(instance.readCursor, length) &&
        ReadRecordData(instance.readCursor, length, data, maxSize, bytesRead))
    {
        RememberCurrentRecord(length);
        read = true;
    }

    return read;
}

static bool VerifyIntegrity(size_t recordStart, uint16_t length)
{
    if (!ReadIntegrityRegion(recordStart, IntegrityRegionSize(length)))
    {
        return false;
    }

    if (!ReadIntegrity(recordStart, length))
    {
        return false;
    }

    return instance.securityPolicy->VerifyIntegrity(IntegrityRegionAddress(), IntegrityRegionSize(length), IntegrityChecksumAddress(length));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- recordStart is a file offset, regionSize is a byte count; distinct semantics
static bool ReadIntegrityRegion(size_t recordStart, uint16_t regionSize)
{
    SolidSyslogFile_SeekTo(instance.readFile, recordStart);
    return ReadExact(IntegrityRegionAddress(), regionSize);
}

static inline bool ReadIntegrity(size_t recordStart, uint16_t dataLength)
{
    SolidSyslogFile_SeekTo(instance.readFile, IntegrityOffset(recordStart, dataLength));
    return ReadExact(IntegrityChecksumAddress(dataLength), instance.securityPolicy->integrity_size);
}

static inline size_t IntegrityOffset(size_t recordStart, uint16_t dataLength)
{
    return recordStart + MAGIC_SIZE + RECORD_LENGTH_SIZE + dataLength;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- recordStart is a file offset, length is a data size; distinct semantics
static bool ReadRecordData(size_t recordStart, uint16_t length, void* data, size_t maxSize, size_t* bytesRead)
{
    size_t copySize = BoundedSize(length, maxSize);

    SolidSyslogFile_SeekTo(instance.readFile, DataOffset(recordStart));

    if (ReadExact(data, copySize))
    {
        *bytesRead = copySize;
    }

    return *bytesRead > 0;
}

static inline size_t BoundedSize(uint16_t length, size_t maxSize)
{
    return (length < maxSize) ? length : maxSize;
}

static inline void RememberCurrentRecord(uint16_t length)
{
    instance.lastSentFlagOffset = SentFlagOffset(instance.readCursor, length);
    instance.hasReadRecord      = true;
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
