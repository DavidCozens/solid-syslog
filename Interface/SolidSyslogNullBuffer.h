#ifndef SOLIDSYSLOGNULLBUFFER_H
#define SOLIDSYSLOGNULLBUFFER_H

#include "SolidSyslogBuffer.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    struct SolidSyslogBuffer* SolidSyslogNullBuffer_Create(struct SolidSyslogSender * sender);
    void                      SolidSyslogNullBuffer_Destroy(struct SolidSyslogBuffer * buffer);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLBUFFER_H */
