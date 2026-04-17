#include "SolidSyslogPosixTcpStream.h"
#include "SolidSyslogStreamDefinition.h"

#include <netinet/tcp.h>
#include <stddef.h>
#include <sys/socket.h>
#include <unistd.h>

enum
{
    INVALID_FD = -1
};

static bool        Open(struct SolidSyslogStream* self, const struct sockaddr_in* addr);
static bool        Send(struct SolidSyslogStream* self, const void* buffer, size_t size);
static void        Close(struct SolidSyslogStream* self);
static void        EnableTcpNoDelay(int fd);
static inline bool IsFileDescriptorValid(int fd);

struct SolidSyslogPosixTcpStream
{
    struct SolidSyslogStream base;
    int                      fd;
};

static struct SolidSyslogPosixTcpStream instance = {.fd = INVALID_FD};

struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(void)
{
    instance.base.Open  = Open;
    instance.base.Send  = Send;
    instance.base.Close = Close;
    return &instance.base;
}

void SolidSyslogPosixTcpStream_Destroy(void)
{
    instance.base.Open  = NULL;
    instance.base.Send  = NULL;
    instance.base.Close = NULL;
}

static bool Open(struct SolidSyslogStream* self, const struct sockaddr_in* addr)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;

    stream->fd = socket(AF_INET, SOCK_STREAM, 0);
    EnableTcpNoDelay(stream->fd);

    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions) -- socket() failure handling deferred to error handling epic
    bool connected = connect(stream->fd, (const struct sockaddr*) addr, sizeof(*addr)) == 0;

    if (!connected)
    {
        close(stream->fd);
        stream->fd = INVALID_FD;
    }

    return connected;
}

static void EnableTcpNoDelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

static bool Send(struct SolidSyslogStream* self, const void* buffer, size_t size)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;
    return send(stream->fd, buffer, size, MSG_NOSIGNAL) >= 0;
}

static void Close(struct SolidSyslogStream* self)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;
    if (IsFileDescriptorValid(stream->fd))
    {
        close(stream->fd);
        stream->fd = INVALID_FD;
    }
}

static inline bool IsFileDescriptorValid(int fd)
{
    return fd >= 0;
}
