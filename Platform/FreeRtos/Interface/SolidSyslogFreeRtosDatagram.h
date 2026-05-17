#ifndef SOLIDSYSLOGFREERTOSDATAGRAM_H
#define SOLIDSYSLOGFREERTOSDATAGRAM_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    enum
    {
        SOLIDSYSLOG_FREE_RTOS_DATAGRAM_SIZE = sizeof(intptr_t) * 8U
    };

    typedef struct
    {
        intptr_t slots[(SOLIDSYSLOG_FREE_RTOS_DATAGRAM_SIZE + sizeof(intptr_t) - 1U) / sizeof(intptr_t)];
    } SolidSyslogFreeRtosDatagramStorage;

    struct SolidSyslogDatagram* SolidSyslogFreeRtosDatagram_Create(SolidSyslogFreeRtosDatagramStorage * storage);
    void SolidSyslogFreeRtosDatagram_Destroy(struct SolidSyslogDatagram * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSDATAGRAM_H */
