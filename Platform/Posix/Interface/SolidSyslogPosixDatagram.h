#ifndef SOLIDSYSLOGPOSIXDATAGRAM_H
#define SOLIDSYSLOGPOSIXDATAGRAM_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogDatagram;

    struct SolidSyslogDatagram* SolidSyslogPosixDatagram_Create(void);
    void SolidSyslogPosixDatagram_Destroy(struct SolidSyslogDatagram * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXDATAGRAM_H */
