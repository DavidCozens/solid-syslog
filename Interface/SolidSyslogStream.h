#ifndef SOLIDSYSLOGSTREAM_H
#define SOLIDSYSLOGSTREAM_H

#include "ExternC.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogStream;

    bool SolidSyslogStream_Open(struct SolidSyslogStream * stream, const struct sockaddr_in* addr);
    bool SolidSyslogStream_Send(struct SolidSyslogStream * stream, const void* buffer, size_t size);
    void SolidSyslogStream_Close(struct SolidSyslogStream * stream);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAM_H */
