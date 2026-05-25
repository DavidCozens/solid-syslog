#ifndef SOLIDSYSLOGSTREAMSENDERERRORS_H
#define SOLIDSYSLOGSTREAMSENDERERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogStreamSenderErrors
    {
        STREAMSENDER_ERROR_NULL_CONFIG,
        STREAMSENDER_ERROR_POOL_EXHAUSTED,
        STREAMSENDER_ERROR_UNKNOWN_DESTROY,
        STREAMSENDER_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource StreamSenderErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAMSENDERERRORS_H */
