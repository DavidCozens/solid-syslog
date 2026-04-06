#ifndef SOLIDSYSLOGUDPSENDER_H
#define SOLIDSYSLOGUDPSENDER_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    struct SolidSyslogUdpSenderConfig
    {
        int (*getPort)(void);         // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
        const char* (*getHost)(void); // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
    };

    struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config);
    void                      SolidSyslogUdpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGUDPSENDER_H */
