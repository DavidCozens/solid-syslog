#ifndef SOLIDSYSLOG_STREAM_SENDER_H
#define SOLIDSYSLOG_STREAM_SENDER_H

#include "SolidSyslogEndpoint.h"
#include "SolidSyslogResolver.h"
#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    struct SolidSyslogStreamSenderConfig
    {
        struct SolidSyslogResolver*        resolver;
        struct SolidSyslogStream*          stream;
        SolidSyslogEndpointFunction        endpoint;        /* fills host/port; called only on (re)connect */
        SolidSyslogEndpointVersionFunction endpointVersion; /* polled cheaply on every Send for stale check */
    };

    struct SolidSyslogSender* SolidSyslogStreamSender_Create(const struct SolidSyslogStreamSenderConfig* config);
    void                      SolidSyslogStreamSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOG_STREAM_SENDER_H */
