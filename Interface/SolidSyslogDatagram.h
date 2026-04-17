#ifndef SOLIDSYSLOGDATAGRAM_H
#define SOLIDSYSLOGDATAGRAM_H

#include "ExternC.h"
#include "SolidSyslogAddress.h"
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    bool SolidSyslogDatagram_Open(struct SolidSyslogDatagram * datagram);
    bool SolidSyslogDatagram_SendTo(struct SolidSyslogDatagram * datagram, const void* buffer, size_t size, const struct SolidSyslogAddress* addr);
    void SolidSyslogDatagram_Close(struct SolidSyslogDatagram * datagram);

EXTERN_C_END

#endif /* SOLIDSYSLOGDATAGRAM_H */
