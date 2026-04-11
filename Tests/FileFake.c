#include "FileFake.h"
#include "SolidSyslogFileApiDefinition.h"
#include "TestAssert.h"

#include <string.h>

enum
{
    FILEFAKE_MAX_SIZE  = 4096,
    FILEFAKE_MAX_FILES = 16,
    FILEFAKE_MAX_PATH  = 128
};

static bool   FileFake_Open(struct SolidSyslogFileApi* self, const char* path);
static void   FileFake_Close(struct SolidSyslogFileApi* self);
static bool   FileFake_IsOpen(struct SolidSyslogFileApi* self);
static bool   FileFake_Read(struct SolidSyslogFileApi* self, void* buf, size_t count);
static bool   FileFake_Write(struct SolidSyslogFileApi* self, const void* buf, size_t count);
static void   FileFake_SeekTo(struct SolidSyslogFileApi* self, size_t offset);
static size_t FileFake_Size(struct SolidSyslogFileApi* self);
static void   FileFake_Truncate(struct SolidSyslogFileApi* self);
static bool   FileFake_Exists(struct SolidSyslogFileApi* self, const char* path);

/* poisoned vtable — installed by Destroy to catch use-after-destroy */
static bool   FileFake_DestroyedOpen(struct SolidSyslogFileApi* self, const char* path);
static void   FileFake_DestroyedClose(struct SolidSyslogFileApi* self);
static bool   FileFake_DestroyedIsOpen(struct SolidSyslogFileApi* self);
static bool   FileFake_DestroyedRead(struct SolidSyslogFileApi* self, void* buf, size_t count);
static bool   FileFake_DestroyedWrite(struct SolidSyslogFileApi* self, const void* buf, size_t count);
static void   FileFake_DestroyedSeekTo(struct SolidSyslogFileApi* self, size_t offset);
static size_t FileFake_DestroyedSize(struct SolidSyslogFileApi* self);
static void   FileFake_DestroyedTruncate(struct SolidSyslogFileApi* self);
static bool   FileFake_DestroyedExists(struct SolidSyslogFileApi* self, const char* path);

/* helpers */
static void              RequireOpenFile(const char* message);
static inline bool       IsFileClosed(void);
static inline bool       HasActiveFile(void);
static inline bool       ShouldFailOnThisCall(bool* flag);
static inline bool       HasBytesToRead(size_t count);
static inline bool       HasSpaceToWrite(size_t count);
static inline void*      ActiveContentAtPosition(void);
static inline void       AdvancePosition(size_t count);
static inline void       ExtendFileSize(void);
static struct FileEntry* FindEntry(const char* path);
static struct FileEntry* FindOrCreateEntry(const char* path);
static struct FileEntry* FindFreeSlot(void);
static inline bool       IsSlotFree(const struct FileEntry* entry);
static inline bool       EntryMatchesPath(const struct FileEntry* entry, const char* path);
static inline void       InitialiseEntry(struct FileEntry* entry, const char* path);

struct FileEntry
{
    char   path[FILEFAKE_MAX_PATH];
    char   content[FILEFAKE_MAX_SIZE];
    size_t fileSize;
    bool   inUse;
};

struct FileFake
{
    struct SolidSyslogFileApi base;
    struct FileEntry          files[FILEFAKE_MAX_FILES];
    struct FileEntry*         active;
    size_t                    position;
    bool                      open;
    bool                      failNextOpen;
    bool                      failNextWrite;
    bool                      failNextRead;
};

static const struct SolidSyslogFileApi LIVE_VTABLE = {
    FileFake_Open,  FileFake_Close, FileFake_IsOpen,   FileFake_Read,
    FileFake_Write, FileFake_SeekTo, FileFake_Size,    FileFake_Truncate, FileFake_Exists,
};

static const struct SolidSyslogFileApi POISONED_VTABLE = {
    FileFake_DestroyedOpen,   FileFake_DestroyedClose, FileFake_DestroyedIsOpen,
    FileFake_DestroyedRead,   FileFake_DestroyedWrite, FileFake_DestroyedSeekTo,
    FileFake_DestroyedSize,   FileFake_DestroyedTruncate, FileFake_DestroyedExists,
};

static struct FileFake instance;

/* ------------------------------------------------------------------
 * Create / Destroy
 * ----------------------------------------------------------------*/

struct SolidSyslogFileApi* FileFake_Create(void)
{
    instance      = (struct FileFake) {0};
    instance.base = LIVE_VTABLE;
    return &instance.base;
}

void FileFake_Destroy(void)
{
    instance      = (struct FileFake) {0};
    instance.base = POISONED_VTABLE;
}

/* ------------------------------------------------------------------
 * Fail injection
 * ----------------------------------------------------------------*/

void FileFake_FailNextOpen(void)
{
    instance.failNextOpen = true;
}

void FileFake_FailNextWrite(void)
{
    instance.failNextWrite = true;
}

void FileFake_FailNextRead(void)
{
    instance.failNextRead = true;
}

/* ------------------------------------------------------------------
 * Inspection
 * ----------------------------------------------------------------*/

const void* FileFake_FileContent(void)
{
    return HasActiveFile() ? instance.active->content : NULL;
}

size_t FileFake_FileSize(void)
{
    return HasActiveFile() ? instance.active->fileSize : 0;
}

static inline bool HasActiveFile(void)
{
    return instance.active != NULL;
}

/* ------------------------------------------------------------------
 * Open
 * ----------------------------------------------------------------*/

static bool FileFake_Open(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;

    if (ShouldFailOnThisCall(&instance.failNextOpen))
    {
        return false;
    }

    struct FileEntry* entry = FindOrCreateEntry(path);
    bool              found = entry != NULL;

    if (found)
    {
        instance.active   = entry;
        instance.open     = true;
        instance.position = 0;
    }

    return found;
}

static inline bool ShouldFailOnThisCall(bool* flag)
{
    bool consumed = *flag;
    *flag         = false;
    return consumed;
}

static struct FileEntry* FindOrCreateEntry(const char* path)
{
    struct FileEntry* entry = FindEntry(path);

    if (entry == NULL)
    {
        entry = FindFreeSlot();

        if (entry != NULL)
        {
            InitialiseEntry(entry, path);
        }
    }

    return entry;
}

static struct FileEntry* FindEntry(const char* path)
{
    struct FileEntry* result = NULL;

    for (size_t i = 0; i < FILEFAKE_MAX_FILES; i++)
    {
        if (EntryMatchesPath(&instance.files[i], path))
        {
            result = &instance.files[i];
            break;
        }
    }

    return result;
}

static inline bool EntryMatchesPath(const struct FileEntry* entry, const char* path)
{
    return entry->inUse && (strcmp(entry->path, path) == 0);
}

static struct FileEntry* FindFreeSlot(void)
{
    struct FileEntry* result = NULL;

    for (size_t i = 0; i < FILEFAKE_MAX_FILES; i++)
    {
        if (IsSlotFree(&instance.files[i]))
        {
            result = &instance.files[i];
            break;
        }
    }

    return result;
}

static inline bool IsSlotFree(const struct FileEntry* entry)
{
    return !entry->inUse;
}

static inline void InitialiseEntry(struct FileEntry* entry, const char* path)
{
    entry->inUse = true;
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- strncpy with bounded size; strncpy_s is not portable
    strncpy(entry->path, path, FILEFAKE_MAX_PATH - 1);
    entry->path[FILEFAKE_MAX_PATH - 1] = '\0';
}

/* ------------------------------------------------------------------
 * Close
 * ----------------------------------------------------------------*/

static void FileFake_Close(struct SolidSyslogFileApi* self)
{
    (void) self;
    RequireOpenFile("Close called with no file open");
    instance.open     = false;
    instance.position = 0;
}

static void RequireOpenFile(const char* message)
{
    if (IsFileClosed())
    {
        TestAssert_Fail(message);
    }
}

static inline bool IsFileClosed(void)
{
    return !instance.open;
}

/* ------------------------------------------------------------------
 * IsOpen
 * ----------------------------------------------------------------*/

static bool FileFake_IsOpen(struct SolidSyslogFileApi* self)
{
    (void) self;
    return instance.open;
}

/* ------------------------------------------------------------------
 * Read
 * ----------------------------------------------------------------*/

static bool FileFake_Read(struct SolidSyslogFileApi* self, void* buf, size_t count)
{
    (void) self;
    RequireOpenFile("Read called with no file open");

    if (ShouldFailOnThisCall(&instance.failNextRead))
    {
        return false;
    }

    bool success = HasBytesToRead(count);

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(buf, ActiveContentAtPosition(), count);
        AdvancePosition(count);
    }

    return success;
}

static inline bool HasBytesToRead(size_t count)
{
    return (instance.position + count) <= instance.active->fileSize;
}

static inline void* ActiveContentAtPosition(void)
{
    return instance.active->content + instance.position;
}

static inline void AdvancePosition(size_t count)
{
    instance.position += count;
}

/* ------------------------------------------------------------------
 * Write
 * ----------------------------------------------------------------*/

static bool FileFake_Write(struct SolidSyslogFileApi* self, const void* buf, size_t count)
{
    (void) self;
    RequireOpenFile("Write called with no file open");

    if (ShouldFailOnThisCall(&instance.failNextWrite))
    {
        return false;
    }

    bool success = HasSpaceToWrite(count);

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(ActiveContentAtPosition(), buf, count);
        AdvancePosition(count);
        ExtendFileSize();
    }

    return success;
}

static inline bool HasSpaceToWrite(size_t count)
{
    return (instance.position + count) <= FILEFAKE_MAX_SIZE;
}

static inline void ExtendFileSize(void)
{
    if (instance.position > instance.active->fileSize)
    {
        instance.active->fileSize = instance.position;
    }
}

/* ------------------------------------------------------------------
 * SeekTo / Size / Truncate / Exists
 * ----------------------------------------------------------------*/

static void FileFake_SeekTo(struct SolidSyslogFileApi* self, size_t offset)
{
    (void) self;
    RequireOpenFile("SeekTo called with no file open");
    instance.position = offset;
}

static size_t FileFake_Size(struct SolidSyslogFileApi* self)
{
    (void) self;
    RequireOpenFile("Size called with no file open");
    return instance.active->fileSize;
}

static void FileFake_Truncate(struct SolidSyslogFileApi* self)
{
    (void) self;
    RequireOpenFile("Truncate called with no file open");
    instance.active->fileSize = 0;
    instance.position         = 0;
}

static bool FileFake_Exists(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    return FindEntry(path) != NULL;
}

/* ------------------------------------------------------------------
 * Poisoned vtable — installed by Destroy
 * ----------------------------------------------------------------*/

static bool FileFake_DestroyedOpen(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    (void) path;
    TestAssert_Fail("Open called after FileFake_Destroy");
    return false;
}

static void FileFake_DestroyedClose(struct SolidSyslogFileApi* self)
{
    (void) self;
    TestAssert_Fail("Close called after FileFake_Destroy");
}

static bool FileFake_DestroyedIsOpen(struct SolidSyslogFileApi* self)
{
    (void) self;
    TestAssert_Fail("IsOpen called after FileFake_Destroy");
    return false;
}

static bool FileFake_DestroyedRead(struct SolidSyslogFileApi* self, void* buf, size_t count)
{
    (void) self;
    (void) buf;
    (void) count;
    TestAssert_Fail("Read called after FileFake_Destroy");
    return false;
}

static bool FileFake_DestroyedWrite(struct SolidSyslogFileApi* self, const void* buf, size_t count)
{
    (void) self;
    (void) buf;
    (void) count;
    TestAssert_Fail("Write called after FileFake_Destroy");
    return false;
}

static void FileFake_DestroyedSeekTo(struct SolidSyslogFileApi* self, size_t offset)
{
    (void) self;
    (void) offset;
    TestAssert_Fail("SeekTo called after FileFake_Destroy");
}

static size_t FileFake_DestroyedSize(struct SolidSyslogFileApi* self)
{
    (void) self;
    TestAssert_Fail("Size called after FileFake_Destroy");
    return 0;
}

static void FileFake_DestroyedTruncate(struct SolidSyslogFileApi* self)
{
    (void) self;
    TestAssert_Fail("Truncate called after FileFake_Destroy");
}

static bool FileFake_DestroyedExists(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    (void) path;
    TestAssert_Fail("Exists called after FileFake_Destroy");
    return false;
}
