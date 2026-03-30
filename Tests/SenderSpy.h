#ifndef SENDERSPY_H
#define SENDERSPY_H

#include "SolidSyslog_Sender.h"

EXTERN_C_BEGIN

    void                       SenderSpy_Reset(void);
    struct SolidSyslog_Sender* SenderSpy_GetSender(void);
    int                        SenderSpy_CallCount(void);
    const char*                SenderSpy_LastBufferAsString(void);

EXTERN_C_END

#endif /* SENDERSPY_H */
