#ifndef SOLIDSYSLOGCIRCULARBUFFERERRORS_H
#define SOLIDSYSLOGCIRCULARBUFFERERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogCircularBufferErrors
    {
        CIRCULARBUFFER_ERROR_POOL_EXHAUSTED,
        CIRCULARBUFFER_ERROR_UNKNOWN_DESTROY,
        CIRCULARBUFFER_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource CircularBufferErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGCIRCULARBUFFERERRORS_H */
