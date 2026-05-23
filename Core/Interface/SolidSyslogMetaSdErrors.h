#ifndef SOLIDSYSLOGMETASDERRORS_H
#define SOLIDSYSLOGMETASDERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogMetaSdErrors
    {
        METASD_ERROR_NULL_CONFIG,
        METASD_ERROR_NULL_COUNTER,
        METASD_ERROR_POOL_EXHAUSTED,
        METASD_ERROR_UNKNOWN_DESTROY,
        METASD_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource MetaSdErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGMETASDERRORS_H */
