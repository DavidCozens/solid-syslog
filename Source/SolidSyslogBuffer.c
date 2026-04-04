#include "SolidSyslogBufferDef.h"

void SolidSyslogBuffer_Write(struct SolidSyslogBuffer* buffer, const void* data, size_t size)
{
    buffer->Write(buffer, data, size);
}

bool SolidSyslogBuffer_Read(struct SolidSyslogBuffer* buffer)
{
    return buffer->Read(buffer);
}
