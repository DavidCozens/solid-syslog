#ifndef SOLIDSYSLOGPOSIXTCPSTREAM_H
#define SOLIDSYSLOGPOSIXTCPSTREAM_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(void);
    void                      SolidSyslogPosixTcpStream_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXTCPSTREAM_H */
