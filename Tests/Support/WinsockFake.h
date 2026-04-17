#ifndef WINSOCKFAKE_H
#define WINSOCKFAKE_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>
#include <winsock2.h>
#include <ws2tcpip.h>

EXTERN_C_BEGIN

    void WinsockFake_Reset(void);

    /* socket configuration */
    void WinsockFake_SetSocketFails(bool fails);

    /* socket accessors */
    int    WinsockFake_SocketCallCount(void);
    SOCKET WinsockFake_SocketFd(void);
    int    WinsockFake_SocketDomain(void);
    int    WinsockFake_SocketType(void);

    /* sendto configuration */
    void WinsockFake_SetSendtoFails(bool fails);

    /* sendto accessors */
    int         WinsockFake_SendtoCallCount(void);
    const char* WinsockFake_LastBufAsString(void);
    size_t      WinsockFake_LastLen(void);
    int         WinsockFake_LastFlags(void);
    int         WinsockFake_LastAddrFamily(void);
    const char* WinsockFake_LastAddrAsString(void);
    int         WinsockFake_LastPort(void);
    int         WinsockFake_LastAddrLen(void);
    SOCKET      WinsockFake_LastSendtoFd(void);

    /* closesocket accessors */
    int    WinsockFake_CloseCallCount(void);
    SOCKET WinsockFake_LastClosedFd(void);

    /* getaddrinfo configuration */
    void WinsockFake_SetGetAddrInfoFails(bool fails);

    /* getaddrinfo accessors */
    int         WinsockFake_GetAddrInfoCallCount(void);
    const char* WinsockFake_LastGetAddrInfoHostname(void);
    int         WinsockFake_LastGetAddrInfoSocktype(void);

    /* freeaddrinfo accessors */
    int WinsockFake_FreeAddrInfoCallCount(void);

    /* Fake Winsock functions — injected into production via UT_PTR_SET. */
    SOCKET WSAAPI WinsockFake_socket(int af, int type, int protocol);
    int WSAAPI    WinsockFake_sendto(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen);
    int WSAAPI    WinsockFake_closesocket(SOCKET s);
    int WSAAPI    WinsockFake_getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res);
    void WSAAPI   WinsockFake_freeaddrinfo(struct addrinfo * res);

EXTERN_C_END

#endif /* WINSOCKFAKE_H */
