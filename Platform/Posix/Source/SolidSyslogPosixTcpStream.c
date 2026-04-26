#include "SolidSyslogPosixTcpStream.h"
#include "SolidSyslogAddressInternal.h"
#include "SolidSyslogMacros.h"
#include "SolidSyslogStreamDefinition.h"

#include <errno.h>
#include <netinet/tcp.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

enum
{
    INVALID_FD           = -1,
    SEND_TIMEOUT_SECONDS = 5
};

static bool             Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr);
static bool             Send(struct SolidSyslogStream* self, const void* buffer, size_t size);
static SolidSyslogSsize Read(struct SolidSyslogStream* self, void* buffer, size_t size);
static void             Close(struct SolidSyslogStream* self);
static void             EnableTcpNoDelay(int fd);
static void             SetSendTimeout(int fd);
static inline bool      IsFileDescriptorValid(int fd);

struct SolidSyslogPosixTcpStream
{
    struct SolidSyslogStream base;
    int                      fd;
};

SOLIDSYSLOG_STATIC_ASSERT(sizeof(struct SolidSyslogPosixTcpStream) <= SOLIDSYSLOG_POSIX_TCP_STREAM_SIZE,
                          "SOLIDSYSLOG_POSIX_TCP_STREAM_SIZE is too small for struct SolidSyslogPosixTcpStream");

static const struct SolidSyslogPosixTcpStream DEFAULT_INSTANCE = {
    {Open, Send, Read, Close},
    INVALID_FD,
};

static const struct SolidSyslogPosixTcpStream DESTROYED_INSTANCE = {
    {NULL, NULL, NULL, NULL},
    INVALID_FD,
};

struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(SolidSyslogPosixTcpStreamStorage* storage)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) storage;
    *stream                                  = DEFAULT_INSTANCE;
    return &stream->base;
}

void SolidSyslogPosixTcpStream_Destroy(struct SolidSyslogStream* stream)
{
    struct SolidSyslogPosixTcpStream* self = (struct SolidSyslogPosixTcpStream*) stream;
    Close(stream);
    *self = DESTROYED_INSTANCE;
}

static bool Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;
    const struct sockaddr_in*         sin    = SolidSyslogAddress_AsConstSockaddrIn(addr);

    stream->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!IsFileDescriptorValid(stream->fd))
    {
        return false;
    }
    EnableTcpNoDelay(stream->fd);
    SetSendTimeout(stream->fd);

    bool connected = connect(stream->fd, (const struct sockaddr*) sin, sizeof(*sin)) == 0;

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

/* Caps blocking time of send() so a wedged peer can't make a single
 * SolidSyslog_Service() call hang. On expiry, send() returns -1 with
 * EAGAIN/EWOULDBLOCK, the Send vtable returns false, and the Service
 * loop closes and reconnects on the next attempt; store-and-forward
 * replays the message on the fresh socket. Hard-coded for now;
 * port-time CMake override is a future option. */
static void SetSendTimeout(int fd)
{
    struct timeval timeout = {.tv_sec = SEND_TIMEOUT_SECONDS, .tv_usec = 0};
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

static bool Send(struct SolidSyslogStream* self, const void* buffer, size_t size)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;
    ssize_t                           sent   = 0;
    /* Retry only on EINTR — portability shim for kernels without SA_RESTART.
     * Any other failure (including EAGAIN/EWOULDBLOCK from SO_SNDTIMEO) and
     * any short return propagates as false; the caller closes and reconnects. */
    do
    {
        sent = send(stream->fd, buffer, size, MSG_NOSIGNAL);
    } while (sent < 0 && errno == EINTR);
    return sent >= 0 && (size_t) sent == size;
}

static SolidSyslogSsize Read(struct SolidSyslogStream* self, void* buffer, size_t size)
{
    struct SolidSyslogPosixTcpStream* stream = (struct SolidSyslogPosixTcpStream*) self;
    return (SolidSyslogSsize) recv(stream->fd, buffer, size, 0);
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
