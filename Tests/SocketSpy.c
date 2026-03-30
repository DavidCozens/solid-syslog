#include "SocketSpy.h"
#include <sys/socket.h>
#include <netinet/in.h>

static int sendtoCallCount;

void SocketSpy_Reset(void)
{
    sendtoCallCount = 0;
}

int SocketSpy_SendtoCallCount(void)
{
    return sendtoCallCount;
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
    return (ssize_t) len;
}

int close(int fd)
{
    (void) fd;
    return 0;
}
