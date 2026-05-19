#ifndef SOLIDSYSLOGPOSIXTCPSTREAM_H
#define SOLIDSYSLOGPOSIXTCPSTREAM_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream;

    struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(void);
    void SolidSyslogPosixTcpStream_Destroy(struct SolidSyslogStream * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXTCPSTREAM_H */
