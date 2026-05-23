#ifndef SOLIDSYSLOGWINDOWSMUTEXERRORS_H
#define SOLIDSYSLOGWINDOWSMUTEXERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogWindowsMutexErrors
    {
        WINDOWSMUTEX_ERROR_POOL_EXHAUSTED,
        WINDOWSMUTEX_ERROR_UNKNOWN_DESTROY,
        WINDOWSMUTEX_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WindowsMutexErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINDOWSMUTEXERRORS_H */
