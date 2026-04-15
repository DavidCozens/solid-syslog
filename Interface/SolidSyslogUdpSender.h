#ifndef SOLIDSYSLOGUDPSENDER_H
#define SOLIDSYSLOGUDPSENDER_H

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
        int (*getPort)(void);         // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
        const char* (*getHost)(void); // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
    };

    struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config);
    void                      SolidSyslogUdpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGUDPSENDER_H */
