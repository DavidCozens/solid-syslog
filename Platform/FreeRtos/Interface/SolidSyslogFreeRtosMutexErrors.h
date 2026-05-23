#ifndef SOLIDSYSLOGFREERTOSMUTEXERRORS_H
#define SOLIDSYSLOGFREERTOSMUTEXERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogFreeRtosMutexErrors
    {
        FREERTOSMUTEX_ERROR_POOL_EXHAUSTED,
        FREERTOSMUTEX_ERROR_UNKNOWN_DESTROY,
        FREERTOSMUTEX_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource FreeRtosMutexErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSMUTEXERRORS_H */
