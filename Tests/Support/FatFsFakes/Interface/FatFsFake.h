#ifndef FATFSFAKE_H
#define FATFSFAKE_H

#include "ExternC.h"
#include "ff.h"

EXTERN_C_BEGIN

    void FatFsFake_Reset(void);

    /* f_open */
    void          FatFsFake_SetOpenResult(FRESULT result);
    int           FatFsFake_OpenCallCount(void);
    const char*   FatFsFake_LastOpenPath(void);
    unsigned char FatFsFake_LastOpenMode(void);

    /* f_close */
    int FatFsFake_CloseCallCount(void);

EXTERN_C_END

#endif /* FATFSFAKE_H */
