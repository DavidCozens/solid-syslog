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
        struct SolidSyslogResolver* resolver;
        struct SolidSyslogDatagram* datagram;
        SolidSyslogEndpointFunction endpoint; /* optional during cut-over; NULL falls back to legacy resolver path */
    };

    struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config);
    void                      SolidSyslogUdpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGUDPSENDER_H */
