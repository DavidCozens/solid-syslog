#ifndef SOLIDSYSLOGPOSIXTCPSTREAM_H
#define SOLIDSYSLOGPOSIXTCPSTREAM_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_TCP_DEFAULT_PORT = 514 /* RFC 6587 convention — same as UDP; no IANA assignment for plain TCP syslog */
    };

    struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(void);
    void                      SolidSyslogPosixTcpStream_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXTCPSTREAM_H */
