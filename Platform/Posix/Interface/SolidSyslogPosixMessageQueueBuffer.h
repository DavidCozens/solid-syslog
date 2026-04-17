#ifndef SOLIDSYSLOGPOSIXMESSAGEQUEUEBUFFER_H
#define SOLIDSYSLOGPOSIXMESSAGEQUEUEBUFFER_H

#include "SolidSyslogBuffer.h"

#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogBuffer* SolidSyslogPosixMessageQueueBuffer_Create(size_t maxMessageSize, long maxMessages);
    void                      SolidSyslogPosixMessageQueueBuffer_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXMESSAGEQUEUEBUFFER_H */
