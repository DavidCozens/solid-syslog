// NOLINTBEGIN(performance-no-int-to-ptr) -- FREERTOS_INVALID_SOCKET is ((Socket_t)~0U) from FreeRTOS-Plus-TCP; the int-to-ptr cast is intrinsic to the upstream
// sentinel and unavoidable.

#include "SolidSyslogFreeRtosDatagram.h"

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

#include "SolidSyslogAddressInternal.h"
#include "SolidSyslogDatagramDefinition.h"
#include "SolidSyslogMacros.h"
#include "SolidSyslogUdpPayload.h"

typedef struct SolidSyslogFreeRtosDatagram FreeRtosDatagram;

struct SolidSyslogFreeRtosDatagram
{
    struct SolidSyslogDatagram base;
    Socket_t                   socket;
};

SOLIDSYSLOG_STATIC_ASSERT(sizeof(FreeRtosDatagram) <= SOLIDSYSLOG_FREERTOSDATAGRAM_SIZE,
                          "SOLIDSYSLOG_FREERTOSDATAGRAM_SIZE is too small for SolidSyslogFreeRtosDatagram layout");

static bool                               FreeRtosDatagram_Open(struct SolidSyslogDatagram* self);
static enum SolidSyslogDatagramSendResult FreeRtosDatagram_SendTo(struct SolidSyslogDatagram* self, const void* buffer, size_t size,
                                                                  const struct SolidSyslogAddress* addr);
static size_t                             FreeRtosDatagram_MaxPayload(struct SolidSyslogDatagram* self);
static void                               FreeRtosDatagram_Close(struct SolidSyslogDatagram* self);
static inline FreeRtosDatagram*           FreeRtosDatagram_From(struct SolidSyslogDatagram* self);
static inline bool                        FreeRtosDatagram_IsOpen(const FreeRtosDatagram* datagram);

static const FreeRtosDatagram DEFAULT_INSTANCE = {
    {FreeRtosDatagram_Open, FreeRtosDatagram_SendTo, FreeRtosDatagram_MaxPayload, FreeRtosDatagram_Close},
    FREERTOS_INVALID_SOCKET,
};

static const FreeRtosDatagram DESTROYED_INSTANCE = {
    {NULL, NULL, NULL, NULL},
    FREERTOS_INVALID_SOCKET,
};

struct SolidSyslogDatagram* SolidSyslogFreeRtosDatagram_Create(SolidSyslogFreeRtosDatagramStorage* storage)
{
    FreeRtosDatagram* datagram = (FreeRtosDatagram*) storage;
    *datagram                  = DEFAULT_INSTANCE;
    return &datagram->base;
}

void SolidSyslogFreeRtosDatagram_Destroy(struct SolidSyslogDatagram* datagram)
{
    FreeRtosDatagram* self = FreeRtosDatagram_From(datagram);
    FreeRtosDatagram_Close(datagram);
    *self = DESTROYED_INSTANCE;
}

static inline FreeRtosDatagram* FreeRtosDatagram_From(struct SolidSyslogDatagram* self)
{
    return (FreeRtosDatagram*) self;
}

static bool FreeRtosDatagram_Open(struct SolidSyslogDatagram* self)
{
    FreeRtosDatagram* datagram = FreeRtosDatagram_From(self);
    if (!FreeRtosDatagram_IsOpen(datagram))
    {
        datagram->socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);
    }
    return FreeRtosDatagram_IsOpen(datagram);
}

static enum SolidSyslogDatagramSendResult FreeRtosDatagram_SendTo(struct SolidSyslogDatagram* self, const void* buffer, size_t size,
                                                                  const struct SolidSyslogAddress* addr)
{
    FreeRtosDatagram*                  datagram = FreeRtosDatagram_From(self);
    enum SolidSyslogDatagramSendResult result   = SOLIDSYSLOG_DATAGRAM_FAILED;
    if (FreeRtosDatagram_IsOpen(datagram))
    {
        const struct freertos_sockaddr* dest = SolidSyslogAddress_AsConstFreertosSockaddr(addr);
        int32_t                         sent = FreeRTOS_sendto(datagram->socket, buffer, size, 0, dest, sizeof(*dest));
        if (sent > 0)
        {
            result = SOLIDSYSLOG_DATAGRAM_SENT;
        }
    }
    return result;
}

static inline bool FreeRtosDatagram_IsOpen(const FreeRtosDatagram* datagram)
{
    return datagram->socket != FREERTOS_INVALID_SOCKET;
}

static size_t FreeRtosDatagram_MaxPayload(struct SolidSyslogDatagram* self)
{
    (void) self;
    return SOLIDSYSLOG_UDP_IPV6_SAFE_PAYLOAD;
}

static void FreeRtosDatagram_Close(struct SolidSyslogDatagram* self)
{
    FreeRtosDatagram* datagram = FreeRtosDatagram_From(self);
    if (FreeRtosDatagram_IsOpen(datagram))
    {
        (void) FreeRTOS_closesocket(datagram->socket);
        datagram->socket = FREERTOS_INVALID_SOCKET;
    }
}

// NOLINTEND(performance-no-int-to-ptr)
