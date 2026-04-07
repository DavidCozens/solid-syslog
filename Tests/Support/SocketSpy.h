#ifndef SOCKETSPY_H
#define SOCKETSPY_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>

EXTERN_C_BEGIN

    void SocketSpy_Reset(void);

    /* sendto configuration */
    void SocketSpy_SetSendtoFails(bool fails);

    /* sendto accessors */
    int         SocketSpy_SendtoCallCount(void);
    const void* SocketSpy_LastBuf(void);
    const char* SocketSpy_LastBufAsString(void);
    size_t      SocketSpy_LastLen(void);
    int         SocketSpy_LastFlags(void);
    int         SocketSpy_LastAddrFamily(void);
    const char* SocketSpy_LastAddrAsString(void);
    int         SocketSpy_LastPort(void);
    socklen_t   SocketSpy_LastAddrLen(void);
    int         SocketSpy_LastSendtoFd(void);

    /* socket accessors */
    int SocketSpy_SocketCallCount(void);
    int SocketSpy_SocketFd(void);
    int SocketSpy_SocketDomain(void);
    int SocketSpy_SocketType(void);

    /* close accessors */
    int SocketSpy_CloseCallCount(void);
    int SocketSpy_LastClosedFd(void);

    /* getaddrinfo accessors */
    int         SocketSpy_GetAddrInfoCallCount(void);
    const char* SocketSpy_LastGetAddrInfoHostname(void);

EXTERN_C_END

#endif /* SOCKETSPY_H */
