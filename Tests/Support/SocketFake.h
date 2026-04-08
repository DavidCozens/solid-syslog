#ifndef SOCKETFAKE_H
#define SOCKETFAKE_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>

EXTERN_C_BEGIN

    void SocketFake_Reset(void);

    /* sendto configuration */
    void SocketFake_SetSendtoFails(bool fails);

    /* sendto accessors */
    int         SocketFake_SendtoCallCount(void);
    const void* SocketFake_LastBuf(void);
    const char* SocketFake_LastBufAsString(void);
    size_t      SocketFake_LastLen(void);
    int         SocketFake_LastFlags(void);
    int         SocketFake_LastAddrFamily(void);
    const char* SocketFake_LastAddrAsString(void);
    int         SocketFake_LastPort(void);
    socklen_t   SocketFake_LastAddrLen(void);
    int         SocketFake_LastSendtoFd(void);

    /* socket accessors */
    int SocketFake_SocketCallCount(void);
    int SocketFake_SocketFd(void);
    int SocketFake_SocketDomain(void);
    int SocketFake_SocketType(void);

    /* close accessors */
    int SocketFake_CloseCallCount(void);
    int SocketFake_LastClosedFd(void);

    /* getaddrinfo accessors */
    int         SocketFake_GetAddrInfoCallCount(void);
    const char* SocketFake_LastGetAddrInfoHostname(void);

EXTERN_C_END

#endif /* SOCKETFAKE_H */
