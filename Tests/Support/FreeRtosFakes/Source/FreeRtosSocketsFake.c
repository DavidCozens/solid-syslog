// NOLINTBEGIN(performance-no-int-to-ptr,bugprone-easily-swappable-parameters) -- API shape (sentinel cast + same-type params) is dictated by FreeRTOS-Plus-TCP

#include "FreeRtosSocketsFake.h"

static unsigned   socketCallCount    = 0;
static BaseType_t lastSocketDomain   = 0;
static BaseType_t lastSocketType     = 0;
static BaseType_t lastSocketProto    = 0;
static Socket_t   lastSocketReturned = NULL;
static bool       socketFails        = false;

static unsigned                        sendtoCallCount             = 0;
static Socket_t                        lastSendtoSocket            = NULL;
static const void*                     lastSendtoBuffer            = NULL;
static size_t                          lastSendtoLength            = 0;
static BaseType_t                      lastSendtoFlags             = 0;
static const struct freertos_sockaddr* lastSendtoDestination       = NULL;
static socklen_t                       lastSendtoDestinationLength = 0;
static bool                            sendtoFails                 = false;

static unsigned closesocketCallCount  = 0;
static Socket_t lastClosesocketSocket = NULL;

/* Sentinel used as a "valid" Socket_t return — non-NULL and not
 * FREERTOS_INVALID_SOCKET. Adapters only inspect Open's return for
 * the invalid sentinel; any other non-zero pointer is treated as success. */
static int fakeSocketHandleAnchor = 0;
#define FAKE_VALID_SOCKET ((Socket_t) & fakeSocketHandleAnchor)

void FreeRtosSocketsFake_Reset(void)
{
    socketCallCount    = 0;
    lastSocketDomain   = 0;
    lastSocketType     = 0;
    lastSocketProto    = 0;
    lastSocketReturned = NULL;
    socketFails        = false;

    sendtoCallCount             = 0;
    lastSendtoSocket            = NULL;
    lastSendtoBuffer            = NULL;
    lastSendtoLength            = 0;
    lastSendtoFlags             = 0;
    lastSendtoDestination       = NULL;
    lastSendtoDestinationLength = 0;
    sendtoFails                 = false;

    closesocketCallCount  = 0;
    lastClosesocketSocket = NULL;
}

void FreeRtosSocketsFake_SetSocketFails(bool fails)
{
    socketFails = fails;
}

void FreeRtosSocketsFake_SetSendtoFails(bool fails)
{
    sendtoFails = fails;
}

unsigned FreeRtosSocketsFake_SocketCallCount(void)
{
    return socketCallCount;
}

BaseType_t FreeRtosSocketsFake_LastSocketDomain(void)
{
    return lastSocketDomain;
}

BaseType_t FreeRtosSocketsFake_LastSocketType(void)
{
    return lastSocketType;
}

BaseType_t FreeRtosSocketsFake_LastSocketProtocol(void)
{
    return lastSocketProto;
}

Socket_t FreeRtosSocketsFake_LastSocketReturned(void)
{
    return lastSocketReturned;
}

unsigned FreeRtosSocketsFake_SendtoCallCount(void)
{
    return sendtoCallCount;
}

Socket_t FreeRtosSocketsFake_LastSendtoSocket(void)
{
    return lastSendtoSocket;
}

const void* FreeRtosSocketsFake_LastSendtoBuffer(void)
{
    return lastSendtoBuffer;
}

size_t FreeRtosSocketsFake_LastSendtoLength(void)
{
    return lastSendtoLength;
}

BaseType_t FreeRtosSocketsFake_LastSendtoFlags(void)
{
    return lastSendtoFlags;
}

const struct freertos_sockaddr* FreeRtosSocketsFake_LastSendtoDestination(void)
{
    return lastSendtoDestination;
}

socklen_t FreeRtosSocketsFake_LastSendtoDestinationLength(void)
{
    return lastSendtoDestinationLength;
}

Socket_t FreeRTOS_socket(BaseType_t xDomain, BaseType_t xType, BaseType_t xProtocol)
{
    ++socketCallCount;
    lastSocketDomain   = xDomain;
    lastSocketType     = xType;
    lastSocketProto    = xProtocol;
    lastSocketReturned = socketFails ? FREERTOS_INVALID_SOCKET : FAKE_VALID_SOCKET;
    return lastSocketReturned;
}

int32_t FreeRTOS_sendto(Socket_t xSocket, const void* pvBuffer, size_t uxTotalDataLength, BaseType_t xFlags,
                        const struct freertos_sockaddr* pxDestinationAddress, socklen_t xDestinationAddressLength)
{
    ++sendtoCallCount;
    lastSendtoSocket            = xSocket;
    lastSendtoBuffer            = pvBuffer;
    lastSendtoLength            = uxTotalDataLength;
    lastSendtoFlags             = xFlags;
    lastSendtoDestination       = pxDestinationAddress;
    lastSendtoDestinationLength = xDestinationAddressLength;
    return sendtoFails ? -pdFREERTOS_ERRNO_ENOBUFS : (int32_t) uxTotalDataLength;
}

unsigned FreeRtosSocketsFake_ClosesocketCallCount(void)
{
    return closesocketCallCount;
}

Socket_t FreeRtosSocketsFake_LastClosesocketSocket(void)
{
    return lastClosesocketSocket;
}

BaseType_t FreeRTOS_closesocket(Socket_t xSocket)
{
    ++closesocketCallCount;
    lastClosesocketSocket = xSocket;
    return pdPASS;
}

// NOLINTEND(performance-no-int-to-ptr,bugprone-easily-swappable-parameters)
