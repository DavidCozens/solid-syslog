#ifndef SOLIDSYSLOGDATAGRAM_H
#define SOLIDSYSLOGDATAGRAM_H

#include <stdbool.h>
#include <stddef.h>

#include "ExternC.h"

struct SolidSyslogAddress;

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    /* Distinct outcomes of SendTo. Oversize is reserved for the EMSGSIZE
     * recovery path (S12.12) — implementations that cannot detect oversize
     * collapse it into Failed. */
    enum SolidSyslogDatagramSendResult
    {
        SOLIDSYSLOG_DATAGRAM_SEND_RESULT_SENT,
        SOLIDSYSLOG_DATAGRAM_SEND_RESULT_OVERSIZE,
        SOLIDSYSLOG_DATAGRAM_SEND_RESULT_FAILED
    };

    bool SolidSyslogDatagram_Open(struct SolidSyslogDatagram * datagram);
    enum SolidSyslogDatagramSendResult SolidSyslogDatagram_SendTo(
        struct SolidSyslogDatagram * datagram,
        const void* buffer,
        size_t size,
        const struct SolidSyslogAddress* addr
    );
    size_t SolidSyslogDatagram_MaxPayload(struct SolidSyslogDatagram * datagram);
    void SolidSyslogDatagram_Close(struct SolidSyslogDatagram * datagram);

EXTERN_C_END

#endif /* SOLIDSYSLOGDATAGRAM_H */
