#ifndef SPYSENDER_H
#define SPYSENDER_H

#include "SolidSyslog_Sender.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void                       SpySender_Reset(void);
    struct SolidSyslog_Sender* SpySender_GetSender(void);
    int                        SpySender_CallCount(void);
    const char*                SpySender_LastBufferAsString(void);

#ifdef __cplusplus
}
#endif

#endif /* SPYSENDER_H */
