#ifndef SOLIDSYSLOGWINSOCKTCPSTREAMERRORS_H
#define SOLIDSYSLOGWINSOCKTCPSTREAMERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogWinsockTcpStreamErrors
    {
        WINSOCKTCPSTREAM_ERROR_POOL_EXHAUSTED,
        WINSOCKTCPSTREAM_ERROR_UNKNOWN_DESTROY,
        WINSOCKTCPSTREAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WinsockTcpStreamErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINSOCKTCPSTREAMERRORS_H */
