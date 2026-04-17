#include "WinsockFake.h"
#include "SafeString.h"
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

enum
{
    WINSOCKFAKE_MAX_BUFFER_SIZE   = 2048,
    WINSOCKFAKE_MAX_HOSTNAME_SIZE = 256
};

static bool               sendtoFails;
static int                sendtoCallCount;
static char               lastBufCopy[WINSOCKFAKE_MAX_BUFFER_SIZE];
static size_t             lastLen;
static int                lastFlags;
static struct sockaddr_in lastAddr;
static int                lastAddrLen;
static SOCKET             lastSendtoFd;

static bool   socketFails;
static int    socketCallCount;
static SOCKET socketFd;
static int    lastSocketDomain;
static int    lastSocketType;

static int    closeCallCount;
static SOCKET lastClosedFd;

static char lastAddrString[INET_ADDRSTRLEN];

static bool               getAddrInfoFails;
static int                getAddrInfoCallCount;
static char               lastGetAddrInfoHostname[WINSOCKFAKE_MAX_HOSTNAME_SIZE];
static int                lastGetAddrInfoSocktype;
static struct sockaddr_in fakeResolvedAddr;
static struct addrinfo    fakeAddrInfo;
static int                freeAddrInfoCallCount;

void WinsockFake_Reset(void)
{
    sendtoFails     = false;
    sendtoCallCount = 0;
    lastBufCopy[0]  = '\0';
    lastLen         = 0;
    lastFlags       = 0;
    lastAddr        = (struct sockaddr_in) {0};
    lastAddrLen     = 0;
    lastSendtoFd    = INVALID_SOCKET;

    socketFails      = false;
    socketCallCount  = 0;
    socketFd         = INVALID_SOCKET;
    lastSocketDomain = 0;
    lastSocketType   = 0;

    closeCallCount = 0;
    lastClosedFd   = INVALID_SOCKET;

    lastAddrString[0] = '\0';

    getAddrInfoFails            = false;
    getAddrInfoCallCount        = 0;
    lastGetAddrInfoHostname[0]  = '\0';
    lastGetAddrInfoSocktype     = 0;
    freeAddrInfoCallCount       = 0;
    fakeResolvedAddr            = (struct sockaddr_in) {0};
    fakeResolvedAddr.sin_family = AF_INET;
    fakeAddrInfo                = (struct addrinfo) {0};
    fakeAddrInfo.ai_family      = AF_INET;
    fakeAddrInfo.ai_addr        = (struct sockaddr*) &fakeResolvedAddr;
    fakeAddrInfo.ai_addrlen     = sizeof(fakeResolvedAddr);
}

/* socket configuration */

void WinsockFake_SetSocketFails(bool fails)
{
    socketFails = fails;
}

/* socket accessors */

int WinsockFake_SocketCallCount(void)
{
    return socketCallCount;
}

SOCKET WinsockFake_SocketFd(void)
{
    return socketFd;
}

int WinsockFake_SocketDomain(void)
{
    return lastSocketDomain;
}

int WinsockFake_SocketType(void)
{
    return lastSocketType;
}

/* sendto configuration */

void WinsockFake_SetSendtoFails(bool fails)
{
    sendtoFails = fails;
}

/* sendto accessors */

int WinsockFake_SendtoCallCount(void)
{
    return sendtoCallCount;
}

const char* WinsockFake_LastBufAsString(void)
{
    return lastBufCopy;
}

size_t WinsockFake_LastLen(void)
{
    return lastLen;
}

int WinsockFake_LastFlags(void)
{
    return lastFlags;
}

int WinsockFake_LastAddrFamily(void)
{
    return lastAddr.sin_family;
}

int WinsockFake_LastPort(void)
{
    return ntohs(lastAddr.sin_port);
}

const char* WinsockFake_LastAddrAsString(void)
{
    inet_ntop(AF_INET, &lastAddr.sin_addr, lastAddrString, sizeof(lastAddrString));
    return lastAddrString;
}

int WinsockFake_LastAddrLen(void)
{
    return lastAddrLen;
}

SOCKET WinsockFake_LastSendtoFd(void)
{
    return lastSendtoFd;
}

/* closesocket accessors */

int WinsockFake_CloseCallCount(void)
{
    return closeCallCount;
}

SOCKET WinsockFake_LastClosedFd(void)
{
    return lastClosedFd;
}

/* getaddrinfo configuration */

void WinsockFake_SetGetAddrInfoFails(bool fails)
{
    getAddrInfoFails = fails;
}

/* getaddrinfo accessors */

int WinsockFake_GetAddrInfoCallCount(void)
{
    return getAddrInfoCallCount;
}

const char* WinsockFake_LastGetAddrInfoHostname(void)
{
    return lastGetAddrInfoHostname;
}

int WinsockFake_LastGetAddrInfoSocktype(void)
{
    return lastGetAddrInfoSocktype;
}

/* freeaddrinfo accessors */

int WinsockFake_FreeAddrInfoCallCount(void)
{
    return freeAddrInfoCallCount;
}

/* Winsock fake functions (UT_PTR_SET injection targets) */

SOCKET WSAAPI WinsockFake_socket(int af, int type, int protocol)
{
    (void) protocol;
    socketCallCount++;
    lastSocketDomain = af;
    lastSocketType   = type;
    if (socketFails)
    {
        socketFd = INVALID_SOCKET;
    }
    else
    {
        socketFd = (SOCKET) socketCallCount; /* deterministic fake handle */
    }
    return socketFd;
}

int WSAAPI WinsockFake_sendto(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
{
    sendtoCallCount++;
    lastSendtoFd    = s;
    size_t copySize = (size_t) len < sizeof(lastBufCopy) - 1 ? (size_t) len : sizeof(lastBufCopy) - 1;
    memcpy(lastBufCopy, buf, copySize);
    lastBufCopy[copySize] = '\0';
    lastLen               = (size_t) len;
    lastFlags             = flags;
    lastAddr              = *(const struct sockaddr_in*) to;
    lastAddrLen           = tolen;
    return sendtoFails ? SOCKET_ERROR : len;
}

int WSAAPI WinsockFake_closesocket(SOCKET s)
{
    closeCallCount++;
    lastClosedFd = s;
    return 0;
}

int WSAAPI WinsockFake_getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
    (void) service;
    getAddrInfoCallCount++;
    lastGetAddrInfoSocktype = hints ? hints->ai_socktype : 0;
    SafeString_Copy(lastGetAddrInfoHostname, sizeof(lastGetAddrInfoHostname), node ? node : "");
    if (getAddrInfoFails)
    {
        return EAI_FAIL;
    }
    inet_pton(AF_INET, node, &fakeResolvedAddr.sin_addr);
    *res = &fakeAddrInfo;
    return 0;
}

void WSAAPI WinsockFake_freeaddrinfo(struct addrinfo* res)
{
    (void) res;
    freeAddrInfoCallCount++;
}
