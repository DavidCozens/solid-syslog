#ifndef SOLIDSYSLOGPASSTHROUGHBUFFERERRORS_H
#define SOLIDSYSLOGPASSTHROUGHBUFFERERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogPassthroughBufferErrors
    {
        PASSTHROUGHBUFFER_ERROR_POOL_EXHAUSTED,
        PASSTHROUGHBUFFER_ERROR_UNKNOWN_DESTROY,
        PASSTHROUGHBUFFER_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PassthroughBufferErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPASSTHROUGHBUFFERERRORS_H */
