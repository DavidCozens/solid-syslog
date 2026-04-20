#include "SolidSyslogStreamDefinition.h"

bool SolidSyslogStream_Open(struct SolidSyslogStream* stream, const struct SolidSyslogAddress* addr)
{
    return stream->Open(stream, addr);
}

bool SolidSyslogStream_Send(struct SolidSyslogStream* stream, const void* buffer, size_t size)
{
    return stream->Send(stream, buffer, size);
}

ssize_t SolidSyslogStream_Read(struct SolidSyslogStream* stream, void* buffer, size_t size)
{
    return stream->Read(stream, buffer, size);
}

void SolidSyslogStream_Close(struct SolidSyslogStream* stream)
{
    stream->Close(stream);
}
