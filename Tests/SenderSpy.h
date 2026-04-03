#ifndef SENDERSPY_H
#define SENDERSPY_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    void                      SenderSpy_Reset(void);
    struct SolidSyslogSender* SenderSpy_GetSender(void);
    int                       SenderSpy_CallCount(void);
    const char*               SenderSpy_LastBufferAsString(void);
    size_t                    SenderSpy_LastSize(void);

EXTERN_C_END

#endif /* SENDERSPY_H */
