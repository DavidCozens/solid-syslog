#ifndef SOLIDSYSLOGNULLBUFFER_H
#define SOLIDSYSLOGNULLBUFFER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogBuffer;

    struct SolidSyslogBuffer* SolidSyslogNullBuffer_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLBUFFER_H */
