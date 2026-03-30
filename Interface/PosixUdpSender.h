#ifndef POSIXUDPSENDER_H
#define POSIXUDPSENDER_H

#include "SolidSyslog_Sender.h"

EXTERN_C_BEGIN

    struct PosixUdpSender_Config
    {
        int placeholder;
    };

    struct SolidSyslog_Sender* PosixUdpSender_Create(const struct PosixUdpSender_Config* config);
    void                       PosixUdpSender_Destroy(struct SolidSyslog_Sender * sender);

EXTERN_C_END

#endif /* POSIXUDPSENDER_H */
