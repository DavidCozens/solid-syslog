#ifndef SOLIDSYSLOGFREERTOSTCPSTREAM_H
#define SOLIDSYSLOGFREERTOSTCPSTREAM_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream;

    struct SolidSyslogStream* SolidSyslogFreeRtosTcpStream_Create(void);
    void SolidSyslogFreeRtosTcpStream_Destroy(struct SolidSyslogStream * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSTCPSTREAM_H */
