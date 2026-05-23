#ifndef SOLIDSYSLOGWINSOCKRESOLVERERRORS_H
#define SOLIDSYSLOGWINSOCKRESOLVERERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogWinsockResolverErrors
    {
        WINSOCKRESOLVER_ERROR_POOL_EXHAUSTED,
        WINSOCKRESOLVER_ERROR_UNKNOWN_DESTROY,
        WINSOCKRESOLVER_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WinsockResolverErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINSOCKRESOLVERERRORS_H */
