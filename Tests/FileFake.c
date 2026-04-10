#include "FileFake.h"
#include "SolidSyslogFileApiDefinition.h"

#include <string.h>

enum
{
    FILEFAKE_MAX_SIZE = 4096
};

static bool   Open(struct SolidSyslogFileApi* self, const char* path);
static void   Close(struct SolidSyslogFileApi* self);
static bool   IsOpen(struct SolidSyslogFileApi* self);
static bool   Read(struct SolidSyslogFileApi* self, void* buf, size_t count);
static bool   Write(struct SolidSyslogFileApi* self, const void* buf, size_t count);
static void   SeekTo(struct SolidSyslogFileApi* self, size_t offset);
static size_t Size(struct SolidSyslogFileApi* self);
static void   Truncate(struct SolidSyslogFileApi* self);

struct FileFake
{
    struct SolidSyslogFileApi base;
    char                      content[FILEFAKE_MAX_SIZE];
    size_t                    fileSize;
    size_t                    position;
    bool                      open;
    bool                      failNextOpen;
    bool                      failNextWrite;
    bool                      failNextRead;
};

static struct FileFake instance;

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
    return &instance.base;
}

void FileFake_Destroy(void)
{
    instance = (struct FileFake) {0};
}

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

const void* FileFake_FileContent(void)
{
    return instance.content;
}

size_t FileFake_FileSize(void)
{
    return instance.fileSize;
}

static bool Open(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    (void) path;

    if (instance.failNextOpen)
    {
        instance.failNextOpen = false;
        return false;
    }

    instance.open     = true;
    instance.position = 0;
    return true;
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

    bool success = (instance.position + count) <= instance.fileSize;

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(buf, instance.content + instance.position, count);
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

    bool success = (instance.position + count) <= FILEFAKE_MAX_SIZE;

    if (success)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded count; memcpy_s is not portable
        memcpy(instance.content + instance.position, buf, count);
        instance.position += count;

        if (instance.position > instance.fileSize)
        {
            instance.fileSize = instance.position;
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
    return instance.fileSize;
}

static void Truncate(struct SolidSyslogFileApi* self)
{
    (void) self;
    instance.fileSize = 0;
    instance.position = 0;
}
