#ifndef SOLIDSYSLOGUDPSENDER_H
#define SOLIDSYSLOGUDPSENDER_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    struct SolidSyslogUdpSenderConfig
    {
        int placeholder;
    };

    struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config);
    void                      SolidSyslogUdpSender_Destroy(struct SolidSyslogSender * sender);

EXTERN_C_END

#endif /* SOLIDSYSLOGUDPSENDER_H */
