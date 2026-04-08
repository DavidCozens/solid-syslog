#ifndef STRINGFAKE_H
#define STRINGFAKE_H

#include "SolidSyslogConfig.h"

EXTERN_C_BEGIN

    void   StringFake_Reset(void);
    void   StringFake_SetHostname(const char* hostname);
    size_t StringFake_GetHostname(char* buffer, size_t size);
    void   StringFake_SetAppName(const char* appName);
    size_t StringFake_GetAppName(char* buffer, size_t size);
    void   StringFake_SetProcessId(const char* procId);
    size_t StringFake_GetProcessId(char* buffer, size_t size);

EXTERN_C_END

#endif /* STRINGFAKE_H */
