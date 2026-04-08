#ifndef SENDERFAKE_H
#define SENDERFAKE_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    void                      SenderFake_Reset(void);
    struct SolidSyslogSender* SenderFake_GetSender(void);
    int                       SenderFake_CallCount(void);
    const char*               SenderFake_LastBufferAsString(void);
    size_t                    SenderFake_LastSize(void);
    void                      SenderFake_FailNextSend(void);

EXTERN_C_END

#endif /* SENDERFAKE_H */
