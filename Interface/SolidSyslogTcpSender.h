#ifndef SOLIDSYSLOG_TCP_SENDER_H
#define SOLIDSYSLOG_TCP_SENDER_H

#include "SolidSyslogEndpoint.h"
#include "SolidSyslogResolver.h"
#include "SolidSyslogStream.h"

struct SolidSyslogSender;

enum
{
    SOLIDSYSLOG_TCP_DEFAULT_PORT = 514 /* RFC 6587 convention — same as UDP; no IANA assignment for plain TCP syslog */
};

struct SolidSyslogTcpSenderConfig
{
    struct SolidSyslogResolver* resolver;
    struct SolidSyslogStream*   stream;
    SolidSyslogEndpointFunction endpoint; /* optional during cut-over; NULL falls back to legacy resolver path */
};

EXTERN_C_BEGIN

    struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config);
    void                      SolidSyslogTcpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOG_TCP_SENDER_H */
