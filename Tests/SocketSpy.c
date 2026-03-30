#include "SocketSpy.h"
#include <sys/socket.h>
#include <netinet/in.h>

static int                sendtoCallCount;
static const void*        lastBuf;
static size_t             lastLen;
static struct sockaddr_in lastAddr;

void SocketSpy_Reset(void)
{
    sendtoCallCount = 0;
    lastBuf         = NULL;
    lastLen         = 0;
    lastAddr        = (struct sockaddr_in){0};
}

int SocketSpy_SendtoCallCount(void)
{
    return sendtoCallCount;
}

const char* SocketSpy_LastBufAsString(void)
{
    return (const char*) lastBuf;
}

int SocketSpy_LastPort(void)
{
    return ntohs(lastAddr.sin_port);
}

size_t SocketSpy_LastLen(void)
{
    return lastLen;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- POSIX API; signature is fixed
int socket(int domain, int type, int protocol)
{
    (void) domain;
    (void) type;
    (void) protocol;
    return 1;
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name,bugprone-easily-swappable-parameters) -- POSIX API; parameter names differ from glibc internal names
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,
               const struct sockaddr* dest_addr, socklen_t addrlen)
{
    (void) sockfd;
    (void) buf;
    (void) len;
    (void) flags;
    (void) dest_addr;
    (void) addrlen;
    sendtoCallCount++;
    lastBuf  = buf;
    lastLen  = len;
    lastAddr = *(const struct sockaddr_in*) dest_addr;
    return (ssize_t) len;
}

int close(int fd)
{
    (void) fd;
    return 0;
}
