#include "SocketFake.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

enum
{
    SOCKETFAKE_MAX_BUFFER_SIZE = 2048
};

static bool               sendtoFails;
static int                sendtoCallCount;
static char               lastBufCopy[SOCKETFAKE_MAX_BUFFER_SIZE];
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

static int                getAddrInfoCallCount;
static char               lastGetAddrInfoHostname[256];
static struct sockaddr_in fakeResolvedAddr;
static struct addrinfo    fakeAddrInfo;

void SocketFake_Reset(void)
{
    sendtoFails       = false;
    sendtoCallCount   = 0;
    lastBufCopy[0]    = '\0';
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

    getAddrInfoCallCount        = 0;
    lastGetAddrInfoHostname[0]  = '\0';
    fakeResolvedAddr            = (struct sockaddr_in) {0};
    fakeResolvedAddr.sin_family = AF_INET;
    fakeAddrInfo                = (struct addrinfo) {0};
    fakeAddrInfo.ai_family      = AF_INET;
    fakeAddrInfo.ai_addr        = (struct sockaddr*) &fakeResolvedAddr;
    fakeAddrInfo.ai_addrlen     = sizeof(fakeResolvedAddr);
}

void SocketFake_SetSendtoFails(bool fails)
{
    sendtoFails = fails;
}

/* sendto accessors */

int SocketFake_SendtoCallCount(void)
{
    return sendtoCallCount;
}

const void* SocketFake_LastBuf(void)
{
    return lastBufCopy;
}

const char* SocketFake_LastBufAsString(void)
{
    return lastBufCopy;
}

size_t SocketFake_LastLen(void)
{
    return lastLen;
}

int SocketFake_LastFlags(void)
{
    return lastFlags;
}

int SocketFake_LastAddrFamily(void)
{
    return lastAddr.sin_family;
}

int SocketFake_LastPort(void)
{
    return ntohs(lastAddr.sin_port);
}

const char* SocketFake_LastAddrAsString(void)
{
    inet_ntop(AF_INET, &lastAddr.sin_addr, lastAddrString, sizeof(lastAddrString));
    return lastAddrString;
}

socklen_t SocketFake_LastAddrLen(void)
{
    return lastAddrLen;
}

int SocketFake_LastSendtoFd(void)
{
    return lastSendtoFd;
}

/* socket accessors */

int SocketFake_SocketCallCount(void)
{
    return socketCallCount;
}

int SocketFake_SocketFd(void)
{
    return socketFd;
}

int SocketFake_SocketDomain(void)
{
    return lastSocketDomain;
}

int SocketFake_SocketType(void)
{
    return lastSocketType;
}

/* close accessors */

int SocketFake_CloseCallCount(void)
{
    return closeCallCount;
}

int SocketFake_LastClosedFd(void)
{
    return lastClosedFd;
}

/* getaddrinfo accessors */

int SocketFake_GetAddrInfoCallCount(void)
{
    return getAddrInfoCallCount;
}

const char* SocketFake_LastGetAddrInfoHostname(void)
{
    return lastGetAddrInfoHostname;
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
    lastSendtoFd    = sockfd;
    size_t copySize = len < sizeof(lastBufCopy) - 1 ? len : sizeof(lastBufCopy) - 1;
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded copySize; memcpy_s is not portable
    memcpy(lastBufCopy, buf, copySize);
    lastBufCopy[copySize] = '\0';
    lastLen               = len;
    lastFlags             = flags;
    lastAddr              = *(const struct sockaddr_in*) dest_addr;
    lastAddrLen           = addrlen;
    return sendtoFails ? (ssize_t) -1 : (ssize_t) len;
}

int close(int fd)
{
    closeCallCount++;
    lastClosedFd = fd;
    return 0;
}

// clang-format off
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name,bugprone-easily-swappable-parameters) -- POSIX API; parameter names differ from glibc internal names
int getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
// clang-format on
{
    (void) service;
    (void) hints;
    getAddrInfoCallCount++;
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- bounded; null-term below
    strncpy(lastGetAddrInfoHostname, node ? node : "", sizeof(lastGetAddrInfoHostname) - 1);
    lastGetAddrInfoHostname[sizeof(lastGetAddrInfoHostname) - 1] = '\0';
    inet_pton(AF_INET, node, &fakeResolvedAddr.sin_addr);
    *res = &fakeAddrInfo;
    return 0;
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) -- POSIX API; parameter names differ from glibc internal names
void freeaddrinfo(struct addrinfo* res)
{
    (void) res;
}
