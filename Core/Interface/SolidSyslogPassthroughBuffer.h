#ifndef SOLIDSYSLOGPASSTHROUGHBUFFER_H
#define SOLIDSYSLOGPASSTHROUGHBUFFER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;
    struct SolidSyslogBuffer;

    /** The no-queue Buffer: Write sends inline through @p sender, so
     *  SolidSyslog_Log blocks until the send returns and Service has nothing to
     *  drain. Suits single-task setups with no store-and-forward. @p sender must
     *  outlive the buffer. A NULL sender, or an exhausted pool, falls back to the
     *  shared NullBuffer. */
    struct SolidSyslogBuffer* SolidSyslogPassthroughBuffer_Create(struct SolidSyslogSender * sender);
    /** Release the pool slot; does not destroy the injected sender. */
    void SolidSyslogPassthroughBuffer_Destroy(struct SolidSyslogBuffer * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPASSTHROUGHBUFFER_H */
