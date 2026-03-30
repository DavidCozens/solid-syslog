#ifndef POSIXUDPSENDER_H
#define POSIXUDPSENDER_H

#include "SolidSyslog_Sender.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct PosixUdpSender_Config
    {
        int placeholder;
    };

    struct SolidSyslog_Sender* PosixUdpSender_Create(const struct PosixUdpSender_Config* config);
    void                       PosixUdpSender_Destroy(struct SolidSyslog_Sender* sender);

#ifdef __cplusplus
}
#endif

#endif /* POSIXUDPSENDER_H */
