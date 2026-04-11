#include "FileFake.h"
#include "SolidSyslogFileApiDefinition.h"

#include <string.h>

enum
{
    FILEFAKE_MAX_SIZE  = 4096,
    FILEFAKE_MAX_FILES = 16,
    FILEFAKE_MAX_PATH  = 128
};

static bool   Open(struct SolidSyslogFileApi* self, const char* path);
static void   Close(struct SolidSyslogFileApi* self);
static bool   IsOpen(struct SolidSyslogFileApi* self);
static bool   Read(struct SolidSyslogFileApi* self, void* buf, size_t count);
static bool   Write(struct SolidSyslogFileApi* self, const void* buf, size_t count);
static void   SeekTo(struct SolidSyslogFileApi* self, size_t offset);
static size_t Size(struct SolidSyslogFileApi* self);
static void   Truncate(struct SolidSyslogFileApi* self);
static bool   Exists(struct SolidSyslogFileApi* self, const char* path);

/* helpers */
static struct FileEntry* FindEntry(const char* path);
static struct FileEntry* FindOrCreateEntry(const char* path);
static struct FileEntry* FindFreeSlot(void);

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

static struct FileFake instance;

/* ------------------------------------------------------------------
 * Create / Destroy
 * ----------------------------------------------------------------*/

struct SolidSyslogFileApi* FileFake_Create(void)
{
    instance               = (struct FileFake) {0};
    instance.base.Open     = Open;
    instance.base.Close    = Close;
    instance.base.IsOpen   = IsOpen;
    instance.base.Read     = Read;
    instance.base.Write    = Write;
    instance.base.SeekTo   = SeekTo;
    instance.base.Size     = Size;
    instance.base.Truncate = Truncate;
    instance.base.Exists   = Exists;
    return &instance.base;
}

void FileFake_Destroy(void)
{
    instance = (struct FileFake) {0};
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
    return (instance.active != NULL) ? instance.active->content : NULL;
}

size_t FileFake_FileSize(void)
{
    return (instance.active != NULL) ? instance.active->fileSize : 0;
}

/* ------------------------------------------------------------------
 * FileApi vtable
 * ----------------------------------------------------------------*/

static bool Open(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;

    if (instance.failNextOpen)
    {
        instance.failNextOpen = false;
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

static void Close(struct SolidSyslogFileApi* self)
{
    (void) self;
    instance.open     = false;
    instance.position = 0;
}

static bool IsOpen(struct SolidSyslogFileApi* self)
{
    (void) self;
    return instance.open;
}

static bool Read(struct SolidSyslogFileApi* self, void* buf, size_t count)
{
    (void) self;

    if (instance.failNextRead)
    {
        instance.failNextRead = false;
        return false;
    }

    bool success = (instance.active != NULL) && ((instance.position + count) <= instance.active->fileSize);

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(buf, instance.active->content + instance.position, count);
        instance.position += count;
    }

    return success;
}

static bool Write(struct SolidSyslogFileApi* self, const void* buf, size_t count)
{
    (void) self;

    if (instance.failNextWrite)
    {
        instance.failNextWrite = false;
        return false;
    }

    bool success = (instance.active != NULL) && ((instance.position + count) <= FILEFAKE_MAX_SIZE);

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(instance.active->content + instance.position, buf, count);
        instance.position += count;

        if (instance.position > instance.active->fileSize)
        {
            instance.active->fileSize = instance.position;
        }
    }

    return success;
}

static void SeekTo(struct SolidSyslogFileApi* self, size_t offset)
{
    (void) self;
    instance.position = offset;
}

static size_t Size(struct SolidSyslogFileApi* self)
{
    (void) self;
    return (instance.active != NULL) ? instance.active->fileSize : 0;
}

static void Truncate(struct SolidSyslogFileApi* self)
{
    (void) self;

    if (instance.active != NULL)
    {
        instance.active->fileSize = 0;
    }

    instance.position = 0;
}

static bool Exists(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    return FindEntry(path) != NULL;
}

/* ------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------*/

static struct FileEntry* FindEntry(const char* path)
{
    struct FileEntry* result = NULL;

    for (size_t i = 0; i < FILEFAKE_MAX_FILES; i++)
    {
        if (instance.files[i].inUse && (strcmp(instance.files[i].path, path) == 0))
        {
            result = &instance.files[i];
            break;
        }
    }

    return result;
}

static struct FileEntry* FindOrCreateEntry(const char* path)
{
    struct FileEntry* entry = FindEntry(path);

    if (entry == NULL)
    {
        entry = FindFreeSlot();

        if (entry != NULL)
        {
            entry->inUse = true;
            // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- strncpy with bounded size; strncpy_s is not portable
            strncpy(entry->path, path, FILEFAKE_MAX_PATH - 1);
            entry->path[FILEFAKE_MAX_PATH - 1] = '\0';
        }
    }

    return entry;
}

static struct FileEntry* FindFreeSlot(void)
{
    struct FileEntry* result = NULL;

    for (size_t i = 0; i < FILEFAKE_MAX_FILES; i++)
    {
        if (!instance.files[i].inUse)
        {
            result = &instance.files[i];
            break;
        }
    }

    return result;
}
