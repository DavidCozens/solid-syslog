#include "FatFsFake.h"

#include <stddef.h>

#include "ff.h"

/* f_open state */
static int         openCallCount;
static const char* lastOpenPath;
static BYTE        lastOpenMode;
static FRESULT     openResult;

/* f_close state */
static int closeCallCount;

void FatFsFake_Reset(void)
{
    openCallCount  = 0;
    lastOpenPath   = NULL;
    lastOpenMode   = 0;
    openResult     = FR_OK;
    closeCallCount = 0;
}

void FatFsFake_SetOpenResult(FRESULT result)
{
    openResult = result;
}

int FatFsFake_OpenCallCount(void)
{
    return openCallCount;
}

const char* FatFsFake_LastOpenPath(void)
{
    return lastOpenPath;
}

unsigned char FatFsFake_LastOpenMode(void)
{
    return lastOpenMode;
}

FRESULT f_open(FIL* fp, const TCHAR* path, BYTE mode)
{
    (void) fp;
    openCallCount++;
    lastOpenPath = path;
    lastOpenMode = mode;
    return openResult;
}

int FatFsFake_CloseCallCount(void)
{
    return closeCallCount;
}

FRESULT f_close(FIL* fp)
{
    (void) fp;
    closeCallCount++;
    return FR_OK;
}
