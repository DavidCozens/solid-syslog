#ifndef PLUSFATFAKE_H
#define PLUSFATFAKE_H

#include "ExternC.h"

EXTERN_C_BEGIN

    void PlusFatFake_Reset(void);

    /* ff_fopen */
    void PlusFatFake_SetOpenFailsForMode(const char* mode);
    void PlusFatFake_SetOpenAlwaysFails(void);
    int PlusFatFake_OpenCallCount(void);
    const char* PlusFatFake_OpenModeAt(int index);
    const char* PlusFatFake_LastOpenPath(void);

    /* ff_fclose */
    int PlusFatFake_CloseCallCount(void);

    /* ff_fread */
    void PlusFatFake_SetReadSource(const void* bytes, unsigned long count);
    int PlusFatFake_ReadCallCount(void);
    unsigned long PlusFatFake_LastReadSize(void);
    unsigned long PlusFatFake_LastReadItems(void);

    /* ff_fwrite */
    void PlusFatFake_SetWriteIncomplete(void);
    int PlusFatFake_WriteCallCount(void);
    const void* PlusFatFake_LastWriteBytes(void);
    unsigned long PlusFatFake_LastWriteItems(void);

    /* ff_fflush */
    void PlusFatFake_SetFflushFails(void);
    int PlusFatFake_FflushCallCount(void);

EXTERN_C_END

#endif /* PLUSFATFAKE_H */
