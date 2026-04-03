#ifndef SOLIDSYSLOGBUFFERDEF_H
#define SOLIDSYSLOGBUFFERDEF_H

#include "SolidSyslogBuffer.h"

EXTERN_C_BEGIN

    struct SolidSyslogBuffer
    {
        void (*Write)(struct SolidSyslogBuffer* self, const void* data, size_t size);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGBUFFERDEF_H */
