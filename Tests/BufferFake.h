#ifndef BUFFERFAKE_H
#define BUFFERFAKE_H

#include "SolidSyslogBuffer.h"

EXTERN_C_BEGIN

    struct SolidSyslogBuffer* BufferFake_Create(void);
    void                      BufferFake_Destroy(struct SolidSyslogBuffer * buffer);

EXTERN_C_END

#endif /* BUFFERFAKE_H */
