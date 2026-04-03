#ifndef STRINGFAKE_H
#define STRINGFAKE_H

#include "SolidSyslogConfig.h"

EXTERN_C_BEGIN

    void StringFake_Reset(void);
    void StringFake_SetHostname(const char* hostname);
    int  StringFake_GetHostname(char* buffer, size_t size);
    void StringFake_SetAppName(const char* appName);
    int  StringFake_GetAppName(char* buffer, size_t size);
    void StringFake_SetProcId(const char* procId);
    int  StringFake_GetProcId(char* buffer, size_t size);

EXTERN_C_END

#endif /* STRINGFAKE_H */
