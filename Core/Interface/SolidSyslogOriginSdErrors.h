#ifndef SOLIDSYSLOGORIGINSDERRORS_H
#define SOLIDSYSLOGORIGINSDERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogOriginSdErrors
    {
        ORIGINSD_ERROR_POOL_EXHAUSTED,
        ORIGINSD_ERROR_UNKNOWN_DESTROY,
        ORIGINSD_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource OriginSdErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGORIGINSDERRORS_H */
