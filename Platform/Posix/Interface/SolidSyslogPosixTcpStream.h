#ifndef SOLIDSYSLOGPOSIXTCPSTREAM_H
#define SOLIDSYSLOGPOSIXTCPSTREAM_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_TCP_DEFAULT_PORT = 601 /* RFC 6587 §3.2 / IANA assignment for syslog over TCP */
    };

    struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(void);
    void                      SolidSyslogPosixTcpStream_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXTCPSTREAM_H */
