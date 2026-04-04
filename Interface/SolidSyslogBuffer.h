#ifndef SOLIDSYSLOGBUFFER_H
#define SOLIDSYSLOGBUFFER_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogBuffer;

    void SolidSyslogBuffer_Write(struct SolidSyslogBuffer * buffer, const void* data, size_t size);
    bool SolidSyslogBuffer_Read(struct SolidSyslogBuffer * buffer);

EXTERN_C_END

#endif /* SOLIDSYSLOGBUFFER_H */
