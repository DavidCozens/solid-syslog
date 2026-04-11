#include "SolidSyslogPosixFileApi.h"
#include "SolidSyslogFileApiDefinition.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define OWNER_READ_WRITE (S_IRUSR | S_IWUSR)
#define DEFAULT_FILE_PERMISSIONS OWNER_READ_WRITE

enum
{
    INVALID_FD = -1
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

struct SolidSyslogPosixFileApi
{
    struct SolidSyslogFileApi base;
    int                       fd;
};

static struct SolidSyslogPosixFileApi instance;

struct SolidSyslogFileApi* SolidSyslogPosixFileApi_Create(void)
{
    instance.base.Open     = Open;
    instance.base.Close    = Close;
    instance.base.IsOpen   = IsOpen;
    instance.base.Read     = Read;
    instance.base.Write    = Write;
    instance.base.SeekTo   = SeekTo;
    instance.base.Size     = Size;
    instance.base.Truncate = Truncate;
    instance.base.Exists   = Exists;
    instance.fd            = INVALID_FD;
    return &instance.base;
}

void SolidSyslogPosixFileApi_Destroy(void)
{
    if (instance.fd != INVALID_FD)
    {
        close(instance.fd);
    }

    instance.fd            = INVALID_FD;
    instance.base.Open     = NULL;
    instance.base.Close    = NULL;
    instance.base.IsOpen   = NULL;
    instance.base.Read     = NULL;
    instance.base.Write    = NULL;
    instance.base.SeekTo   = NULL;
    instance.base.Size     = NULL;
    instance.base.Truncate = NULL;
    instance.base.Exists   = NULL;
}

static bool Open(struct SolidSyslogFileApi* self, const char* path)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    posix->fd                             = open(path, O_RDWR | O_CREAT, DEFAULT_FILE_PERMISSIONS);
    return posix->fd != INVALID_FD;
}

static void Close(struct SolidSyslogFileApi* self)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;

    if (posix->fd != INVALID_FD)
    {
        close(posix->fd);
        posix->fd = INVALID_FD;
    }
}

static bool IsOpen(struct SolidSyslogFileApi* self)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    return posix->fd != INVALID_FD;
}

static bool Read(struct SolidSyslogFileApi* self, void* buf, size_t count)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    return read(posix->fd, buf, count) == (ssize_t) count;
}

static bool Write(struct SolidSyslogFileApi* self, const void* buf, size_t count)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    return write(posix->fd, buf, count) == (ssize_t) count;
}

static void SeekTo(struct SolidSyslogFileApi* self, size_t offset)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    lseek(posix->fd, (off_t) offset, SEEK_SET);
}

static size_t Size(struct SolidSyslogFileApi* self)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    off_t                           size  = lseek(posix->fd, 0, SEEK_END);
    return (size >= 0) ? (size_t) size : 0;
}

static void Truncate(struct SolidSyslogFileApi* self)
{
    struct SolidSyslogPosixFileApi* posix = (struct SolidSyslogPosixFileApi*) self;
    ftruncate(posix->fd, 0);
}

static bool Exists(struct SolidSyslogFileApi* self, const char* path)
{
    (void) self;
    return access(path, F_OK) == 0;
}
