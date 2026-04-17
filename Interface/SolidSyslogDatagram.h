#ifndef SOLIDSYSLOGDATAGRAM_H
#define SOLIDSYSLOGDATAGRAM_H

#include "ExternC.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    void SolidSyslogDatagram_Open(struct SolidSyslogDatagram* datagram);
    bool SolidSyslogDatagram_SendTo(struct SolidSyslogDatagram* datagram, const void* buffer, size_t size, const struct sockaddr_in* addr);
    void SolidSyslogDatagram_Close(struct SolidSyslogDatagram* datagram);

EXTERN_C_END

#endif /* SOLIDSYSLOGDATAGRAM_H */
