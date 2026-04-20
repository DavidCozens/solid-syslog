#ifndef STREAMFAKE_H
#define STREAMFAKE_H

#include "ExternC.h"

#include <stddef.h>
#include <sys/types.h>

EXTERN_C_BEGIN

    struct SolidSyslogStream;
    struct SolidSyslogAddress;

    struct SolidSyslogStream*        StreamFake_Create(void);
    void                             StreamFake_Destroy(struct SolidSyslogStream* stream);
    int                              StreamFake_OpenCallCount(struct SolidSyslogStream* stream);
    const struct SolidSyslogAddress* StreamFake_LastOpenAddr(struct SolidSyslogStream* stream);
    int                              StreamFake_SendCallCount(struct SolidSyslogStream* stream);
    const void*                      StreamFake_LastSendBuf(struct SolidSyslogStream* stream);
    size_t                           StreamFake_LastSendSize(struct SolidSyslogStream* stream);
    int                              StreamFake_ReadCallCount(struct SolidSyslogStream* stream);
    void*                            StreamFake_LastReadBuf(struct SolidSyslogStream* stream);
    size_t                           StreamFake_LastReadSize(struct SolidSyslogStream* stream);
    void                             StreamFake_SetReadReturn(struct SolidSyslogStream* stream, ssize_t value);
    int                              StreamFake_CloseCallCount(struct SolidSyslogStream* stream);

EXTERN_C_END

#endif /* STREAMFAKE_H */
