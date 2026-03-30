#include "SocketSpy.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static int                sendtoCallCount;
static const void*        lastBuf;
static size_t             lastLen;
static int                lastFlags;
static struct sockaddr_in lastAddr;
static socklen_t          lastAddrLen;
static int                lastSendtoFd;

static int socketCallCount;
static int socketFd;
static int lastSocketDomain;
static int lastSocketType;

static int closeCallCount;
static int lastClosedFd;

static char lastAddrString[INET_ADDRSTRLEN];

void SocketSpy_Reset(void)
{
    sendtoCallCount   = 0;
    lastBuf           = NULL;
    lastLen           = 0;
    lastFlags         = 0;
    lastAddr          = (struct sockaddr_in) {0};
    lastAddrLen       = 0;
    lastSendtoFd      = -1;
    socketCallCount   = 0;
    socketFd          = -1;
    lastSocketDomain  = 0;
    lastSocketType    = 0;
    closeCallCount    = 0;
    lastClosedFd      = -1;
    lastAddrString[0] = '\0';
}

/* sendto accessors */

int SocketSpy_SendtoCallCount(void)
{
    return sendtoCallCount;
}

const void* SocketSpy_LastBuf(void)
{
    return lastBuf;
}

const char* SocketSpy_LastBufAsString(void)
{
    return (const char*) lastBuf;
}

size_t SocketSpy_LastLen(void)
{
    return lastLen;
}

int SocketSpy_LastFlags(void)
{
    return lastFlags;
}

int SocketSpy_LastAddrFamily(void)
{
    return lastAddr.sin_family;
}

int SocketSpy_LastPort(void)
{
    return ntohs(lastAddr.sin_port);
}

const char* SocketSpy_LastAddrAsString(void)
{
    inet_ntop(AF_INET, &lastAddr.sin_addr, lastAddrString, sizeof(lastAddrString));
    return lastAddrString;
}

socklen_t SocketSpy_LastAddrLen(void)
{
    return lastAddrLen;
}

int SocketSpy_LastSendtoFd(void)
{
    return lastSendtoFd;
}

/* socket accessors */

int SocketSpy_SocketCallCount(void)
{
    return socketCallCount;
}

int SocketSpy_SocketFd(void)
{
    return socketFd;
}

int SocketSpy_SocketDomain(void)
{
    return lastSocketDomain;
}

int SocketSpy_SocketType(void)
{
    return lastSocketType;
}

/* close accessors */

int SocketSpy_CloseCallCount(void)
{
    return closeCallCount;
}

int SocketSpy_LastClosedFd(void)
{
    return lastClosedFd;
}

/* POSIX strong-symbol fakes */

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- POSIX API; signature is fixed
int socket(int domain, int type, int protocol)
{
    (void) protocol;
    socketCallCount++;
    lastSocketDomain = domain;
    lastSocketType   = type;
    socketFd         = socketCallCount; /* deterministic fake fd */
    return socketFd;
}

// clang-format off
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name,bugprone-easily-swappable-parameters) -- POSIX API; parameter names differ from glibc internal names
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
// clang-format on
{
    sendtoCallCount++;
    lastSendtoFd = sockfd;
    lastBuf      = buf;
    lastLen      = len;
    lastFlags    = flags;
    lastAddr     = *(const struct sockaddr_in*) dest_addr;
    lastAddrLen  = addrlen;
    return (ssize_t) len;
}

int close(int fd)
{
    closeCallCount++;
    lastClosedFd = fd;
    return 0;
}
