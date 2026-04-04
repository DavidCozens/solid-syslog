#ifndef SOLIDSYSLOGPOSIXMQBUFFER_H
#define SOLIDSYSLOGPOSIXMQBUFFER_H

#include "SolidSyslogBuffer.h"

#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogBuffer* SolidSyslogPosixMqBuffer_Create(size_t maxMessageSize, long maxMessages);
    void                      SolidSyslogPosixMqBuffer_Destroy(struct SolidSyslogBuffer * buffer);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXMQBUFFER_H */
