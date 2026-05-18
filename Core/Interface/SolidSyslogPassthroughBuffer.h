#ifndef SOLIDSYSLOGPASSTHROUGHBUFFER_H
#define SOLIDSYSLOGPASSTHROUGHBUFFER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    struct SolidSyslogBuffer* SolidSyslogPassthroughBuffer_Create(struct SolidSyslogSender * sender);
    void SolidSyslogPassthroughBuffer_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPASSTHROUGHBUFFER_H */
