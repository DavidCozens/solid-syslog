#ifndef SOLIDSYSLOGSTREAM_H
#define SOLIDSYSLOGSTREAM_H

#include "ExternC.h"
#include "SolidSyslogAddress.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

EXTERN_C_BEGIN

    struct SolidSyslogStream;

    bool    SolidSyslogStream_Open(struct SolidSyslogStream * stream, const struct SolidSyslogAddress* addr);
    bool    SolidSyslogStream_Send(struct SolidSyslogStream * stream, const void* buffer, size_t size);
    ssize_t SolidSyslogStream_Read(struct SolidSyslogStream * stream, void* buffer, size_t size);
    void    SolidSyslogStream_Close(struct SolidSyslogStream * stream);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAM_H */
