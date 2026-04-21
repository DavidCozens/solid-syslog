#ifndef SOLIDSYSLOGUDPSENDER_H
#define SOLIDSYSLOGUDPSENDER_H

#include "SolidSyslogDatagram.h"
#include "SolidSyslogEndpoint.h"
#include "SolidSyslogResolver.h"
#include "SolidSyslogSender.h"

enum
{
    SOLIDSYSLOG_UDP_DEFAULT_PORT = 514 /* RFC 5426 */
};

EXTERN_C_BEGIN

    struct SolidSyslogUdpSenderConfig
    {
        struct SolidSyslogResolver*        resolver;
        struct SolidSyslogDatagram*        datagram;
        SolidSyslogEndpointFunction        endpoint;        /* fills host/port; called only on (re)connect */
        SolidSyslogEndpointVersionFunction endpointVersion; /* polled cheaply on every Send for stale check */
    };

    struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config);
    void                      SolidSyslogUdpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGUDPSENDER_H */
