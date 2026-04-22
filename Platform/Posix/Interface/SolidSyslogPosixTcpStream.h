#ifndef SOLIDSYSLOGPOSIXTCPSTREAM_H
#define SOLIDSYSLOGPOSIXTCPSTREAM_H

#include "SolidSyslogStream.h"
#include <stdint.h>

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_TCP_DEFAULT_PORT = 601 /* RFC 6587 §3.2 / IANA assignment for syslog over TCP */
    };

    enum
    {
        SOLIDSYSLOG_POSIX_TCP_STREAM_SIZE = sizeof(intptr_t) * 5
    };

    typedef struct
    {
        intptr_t slots[(SOLIDSYSLOG_POSIX_TCP_STREAM_SIZE + sizeof(intptr_t) - 1) / sizeof(intptr_t)];
    } SolidSyslogPosixTcpStreamStorage;

    struct SolidSyslogStream* SolidSyslogPosixTcpStream_Create(SolidSyslogPosixTcpStreamStorage * storage);
    void                      SolidSyslogPosixTcpStream_Destroy(struct SolidSyslogStream * stream);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXTCPSTREAM_H */
