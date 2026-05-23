#ifndef SOLIDSYSLOGMBEDTLSSTREAMERRORS_H
#define SOLIDSYSLOGMBEDTLSSTREAMERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogMbedTlsStreamErrors
    {
        MBEDTLSSTREAM_ERROR_POOL_EXHAUSTED,
        MBEDTLSSTREAM_ERROR_UNKNOWN_DESTROY,
        MBEDTLSSTREAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource MbedTlsStreamErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGMBEDTLSSTREAMERRORS_H */
