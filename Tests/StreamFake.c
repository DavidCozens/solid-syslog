#include "StreamFake.h"
#include "SolidSyslogStreamDefinition.h"

#include <stdlib.h>

struct StreamFake
{
    struct SolidSyslogStream         base;
    int                              openCallCount;
    const struct SolidSyslogAddress* lastOpenAddr;
    bool                             openFails;
    int                              sendCallCount;
    const void*                      lastSendBuf;
    size_t                           lastSendSize;
    int                              readCallCount;
    void*                            lastReadBuf;
    size_t                           lastReadSize;
    SolidSyslogSsize                 readReturn;
    int                              closeCallCount;
};

static bool Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr)
{
    struct StreamFake* fake = (struct StreamFake*) self;
    fake->openCallCount++;
    fake->lastOpenAddr = addr;
    return !fake->openFails;
}

static bool Send(struct SolidSyslogStream* self, const void* buffer, size_t size)
{
    struct StreamFake* fake = (struct StreamFake*) self;
    fake->sendCallCount++;
    fake->lastSendBuf  = buffer;
    fake->lastSendSize = size;
    return true;
}

static SolidSyslogSsize Read(struct SolidSyslogStream* self, void* buffer, size_t size)
{
    struct StreamFake* fake = (struct StreamFake*) self;
    fake->readCallCount++;
    fake->lastReadBuf  = buffer;
    fake->lastReadSize = size;
    return fake->readReturn;
}

static void Close(struct SolidSyslogStream* self)
{
    struct StreamFake* fake = (struct StreamFake*) self;
    fake->closeCallCount++;
}

struct SolidSyslogStream* StreamFake_Create(void)
{
    struct StreamFake* fake = (struct StreamFake*) calloc(1, sizeof(struct StreamFake));
    fake->base.Open         = Open;
    fake->base.Send         = Send;
    fake->base.Read         = Read;
    fake->base.Close        = Close;
    return &fake->base;
}

void StreamFake_Destroy(struct SolidSyslogStream* stream)
{
    free(stream);
}

int StreamFake_OpenCallCount(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->openCallCount;
}

const struct SolidSyslogAddress* StreamFake_LastOpenAddr(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->lastOpenAddr;
}

int StreamFake_SendCallCount(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->sendCallCount;
}

const void* StreamFake_LastSendBuf(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->lastSendBuf;
}

size_t StreamFake_LastSendSize(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->lastSendSize;
}

int StreamFake_ReadCallCount(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->readCallCount;
}

void* StreamFake_LastReadBuf(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->lastReadBuf;
}

size_t StreamFake_LastReadSize(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->lastReadSize;
}

void StreamFake_SetReadReturn(struct SolidSyslogStream* stream, SolidSyslogSsize value)
{
    ((struct StreamFake*) stream)->readReturn = value;
}

void StreamFake_SetOpenFails(struct SolidSyslogStream* stream, bool fails)
{
    ((struct StreamFake*) stream)->openFails = fails;
}

int StreamFake_CloseCallCount(struct SolidSyslogStream* stream)
{
    return ((struct StreamFake*) stream)->closeCallCount;
}
