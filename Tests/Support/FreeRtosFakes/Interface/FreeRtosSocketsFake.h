#ifndef FREERTOSSOCKETSFAKE_H
#define FREERTOSSOCKETSFAKE_H

#include "ExternC.h"

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

EXTERN_C_BEGIN

    void FreeRtosSocketsFake_Reset(void);

    /* socket configuration */
    void FreeRtosSocketsFake_SetSocketFails(bool fails);

    /* sendto configuration */
    void FreeRtosSocketsFake_SetSendtoFails(bool fails);

    /* socket accessors */
    unsigned   FreeRtosSocketsFake_SocketCallCount(void);
    BaseType_t FreeRtosSocketsFake_LastSocketDomain(void);
    BaseType_t FreeRtosSocketsFake_LastSocketType(void);
    BaseType_t FreeRtosSocketsFake_LastSocketProtocol(void);
    Socket_t   FreeRtosSocketsFake_LastSocketReturned(void);

    /* sendto accessors */
    unsigned                        FreeRtosSocketsFake_SendtoCallCount(void);
    Socket_t                        FreeRtosSocketsFake_LastSendtoSocket(void);
    const void*                     FreeRtosSocketsFake_LastSendtoBuffer(void);
    size_t                          FreeRtosSocketsFake_LastSendtoLength(void);
    BaseType_t                      FreeRtosSocketsFake_LastSendtoFlags(void);
    const struct freertos_sockaddr* FreeRtosSocketsFake_LastSendtoDestination(void);
    socklen_t                       FreeRtosSocketsFake_LastSendtoDestinationLength(void);

    /* closesocket accessors */
    unsigned FreeRtosSocketsFake_ClosesocketCallCount(void);
    Socket_t FreeRtosSocketsFake_LastClosesocketSocket(void);

EXTERN_C_END

#endif /* FREERTOSSOCKETSFAKE_H */
