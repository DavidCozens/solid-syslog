#ifndef SOLIDSYSLOGSTREAMSENDERERRORS_H
#define SOLIDSYSLOGSTREAMSENDERERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogStreamSenderErrors
    {
        STREAMSENDER_ERROR_POOL_EXHAUSTED,
        STREAMSENDER_ERROR_UNKNOWN_DESTROY,
        STREAMSENDER_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource StreamSenderErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAMSENDERERRORS_H */
