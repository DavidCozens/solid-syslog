#ifndef SENDERSPY_H
#define SENDERSPY_H

#include "SolidSyslog_Sender.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void                       SenderSpy_Reset(void);
    struct SolidSyslog_Sender* SenderSpy_GetSender(void);
    int                        SenderSpy_CallCount(void);
    const char*                SenderSpy_LastBufferAsString(void);

#ifdef __cplusplus
}
#endif

#endif /* SENDERSPY_H */
