#ifndef SOLIDSYSLOGPLUSTCPDATAGRAM_H
#define SOLIDSYSLOGPLUSTCPDATAGRAM_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    struct SolidSyslogDatagram* SolidSyslogPlusTcpDatagram_Create(void);
    void SolidSyslogPlusTcpDatagram_Destroy(struct SolidSyslogDatagram * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPLUSTCPDATAGRAM_H */
