#include "SolidSyslogWinsockTcpStream.h"
#include "SolidSyslogAddressInternal.h"
#include "SolidSyslogMacros.h"
#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogWinsockTcpStreamInternal.h"

#include <stddef.h>

/* File-local forwarders. Taking the address of a __declspec(dllimport)
   Winsock function for static initialisation triggers MSVC C4232 (the address
   isn't a compile-time constant); forwarding through a static function whose
   address IS a compile-time constant avoids the warning without a suppression. */
static SOCKET WSAAPI CallSocket(int af, int type, int protocol);
static int WSAAPI    CallConnect(SOCKET s, const struct sockaddr* name, int namelen);
static int WSAAPI    CallSend(SOCKET s, const char* buf, int len, int flags);
static int WSAAPI    CallRecv(SOCKET s, char* buf, int len, int flags);
static int WSAAPI    CallSetSockOpt(SOCKET s, int level, int optname, const char* optval, int optlen);
static int WSAAPI    CallCloseSocket(SOCKET s);

WinsockTcpStreamSocketFn      WinsockTcpStream_socket      = CallSocket;
WinsockTcpStreamConnectFn     WinsockTcpStream_connect     = CallConnect;
WinsockTcpStreamSendFn        WinsockTcpStream_send        = CallSend;
WinsockTcpStreamRecvFn        WinsockTcpStream_recv        = CallRecv;
WinsockTcpStreamSetSockOptFn  WinsockTcpStream_setsockopt  = CallSetSockOpt;
WinsockTcpStreamCloseSocketFn WinsockTcpStream_closesocket = CallCloseSocket;

static SOCKET WSAAPI CallSocket(int af, int type, int protocol)
{
    return socket(af, type, protocol);
}

static int WSAAPI CallConnect(SOCKET s, const struct sockaddr* name, int namelen)
{
    return connect(s, name, namelen);
}

static int WSAAPI CallSend(SOCKET s, const char* buf, int len, int flags)
{
    return send(s, buf, len, flags);
}

static int WSAAPI CallRecv(SOCKET s, char* buf, int len, int flags)
{
    return recv(s, buf, len, flags);
}

static int WSAAPI CallSetSockOpt(SOCKET s, int level, int optname, const char* optval, int optlen)
{
    return setsockopt(s, level, optname, optval, optlen);
}

static int WSAAPI CallCloseSocket(SOCKET s)
{
    return closesocket(s);
}

enum
{
    SEND_TIMEOUT_MILLISECONDS = 5000
};

struct SolidSyslogWinsockTcpStream
{
    struct SolidSyslogStream base;
    SOCKET                   fd;
};

static bool             Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr);
static bool             Send(struct SolidSyslogStream* self, const void* buffer, size_t size);
static SolidSyslogSsize Read(struct SolidSyslogStream* self, void* buffer, size_t size);
static void             Close(struct SolidSyslogStream* self);

static SOCKET      OpenAndConfigureSocket(void);
static void        EnableTcpNoDelay(SOCKET fd);
static void        SetSendTimeout(SOCKET fd);
static inline bool IsSocketValid(SOCKET fd);
static bool        ConnectOrCloseOnFailure(struct SolidSyslogWinsockTcpStream* stream, const struct sockaddr_in* sin);
static bool        Connect(SOCKET fd, const struct sockaddr_in* sin);
static bool        WroteAllBytes(int sent, size_t expected);

SOLIDSYSLOG_STATIC_ASSERT(sizeof(struct SolidSyslogWinsockTcpStream) <= SOLIDSYSLOG_WINSOCK_TCP_STREAM_SIZE,
                          "SOLIDSYSLOG_WINSOCK_TCP_STREAM_SIZE is too small for struct SolidSyslogWinsockTcpStream");

static const struct SolidSyslogWinsockTcpStream DEFAULT_INSTANCE = {
    {Open, Send, Read, Close},
    INVALID_SOCKET,
};

static const struct SolidSyslogWinsockTcpStream DESTROYED_INSTANCE = {
    {NULL, NULL, NULL, NULL},
    INVALID_SOCKET,
};

struct SolidSyslogStream* SolidSyslogWinsockTcpStream_Create(SolidSyslogWinsockTcpStreamStorage* storage)
{
    struct SolidSyslogWinsockTcpStream* stream = (struct SolidSyslogWinsockTcpStream*) storage;
    *stream                                    = DEFAULT_INSTANCE;
    return &stream->base;
}

void SolidSyslogWinsockTcpStream_Destroy(struct SolidSyslogStream* stream)
{
    struct SolidSyslogWinsockTcpStream* self = (struct SolidSyslogWinsockTcpStream*) stream;
    Close(stream);
    *self = DESTROYED_INSTANCE;
}

static bool Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogWinsockTcpStream* stream    = (struct SolidSyslogWinsockTcpStream*) self;
    const struct sockaddr_in*           sin       = SolidSyslogAddress_AsConstSockaddrIn(addr);
    bool                                connected = false;

    stream->fd = OpenAndConfigureSocket();
    if (IsSocketValid(stream->fd))
    {
        connected = ConnectOrCloseOnFailure(stream, sin);
    }
    return connected;
}

static SOCKET OpenAndConfigureSocket(void)
{
    SOCKET fd = WinsockTcpStream_socket(AF_INET, SOCK_STREAM, 0);
    if (IsSocketValid(fd))
    {
        EnableTcpNoDelay(fd);
        SetSendTimeout(fd);
    }
    return fd;
}

static void EnableTcpNoDelay(SOCKET fd)
{
    int enable = 1;
    WinsockTcpStream_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*) &enable, (int) sizeof(enable));
}

/* Caps blocking time of send() so a wedged peer can't make a single
 * SolidSyslog_Service() call hang. On expiry, send() returns SOCKET_ERROR
 * (last error WSAETIMEDOUT), the Send vtable returns false, and the Service
 * loop closes and reconnects on the next attempt; store-and-forward replays
 * the message on the fresh socket. Hard-coded for now; port-time CMake
 * override is a future option. */
static void SetSendTimeout(SOCKET fd)
{
    DWORD timeout = SEND_TIMEOUT_MILLISECONDS;
    WinsockTcpStream_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, (int) sizeof(timeout));
}

static inline bool IsSocketValid(SOCKET fd)
{
    return fd != INVALID_SOCKET;
}

static bool ConnectOrCloseOnFailure(struct SolidSyslogWinsockTcpStream* stream, const struct sockaddr_in* sin)
{
    bool connected = Connect(stream->fd, sin);
    if (!connected)
    {
        WinsockTcpStream_closesocket(stream->fd);
        stream->fd = INVALID_SOCKET;
    }
    return connected;
}

static bool Connect(SOCKET fd, const struct sockaddr_in* sin)
{
    return WinsockTcpStream_connect(fd, (const struct sockaddr*) sin, (int) sizeof(*sin)) != SOCKET_ERROR;
}

static bool Send(struct SolidSyslogStream* self, const void* buffer, size_t size)
{
    struct SolidSyslogWinsockTcpStream* stream = (struct SolidSyslogWinsockTcpStream*) self;
    int                                 sent   = WinsockTcpStream_send(stream->fd, (const char*) buffer, (int) size, 0);
    return WroteAllBytes(sent, size);
}

/* Winsock send() does not have signal-interruption semantics, so the EINTR
 * retry loop present in PosixTcpStream is not needed here. Any failure
 * (including WSAETIMEDOUT from SO_SNDTIMEO firing) and any short return
 * propagate via WroteAllBytes; the caller closes and reconnects, store-
 * and-forward replays the message on the fresh socket. */
static bool WroteAllBytes(int sent, size_t expected)
{
    return (sent >= 0) && ((size_t) sent == expected);
}

static SolidSyslogSsize Read(struct SolidSyslogStream* self, void* buffer, size_t size)
{
    struct SolidSyslogWinsockTcpStream* stream = (struct SolidSyslogWinsockTcpStream*) self;
    return (SolidSyslogSsize) WinsockTcpStream_recv(stream->fd, (char*) buffer, (int) size, 0);
}

static void Close(struct SolidSyslogStream* self)
{
    struct SolidSyslogWinsockTcpStream* stream = (struct SolidSyslogWinsockTcpStream*) self;
    if (IsSocketValid(stream->fd))
    {
        WinsockTcpStream_closesocket(stream->fd);
        stream->fd = INVALID_SOCKET;
    }
}
