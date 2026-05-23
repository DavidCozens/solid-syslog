#ifndef SOLIDSYSLOGTIMEQUALITYSDERRORS_H
#define SOLIDSYSLOGTIMEQUALITYSDERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogTimeQualitySdErrors
    {
        TIMEQUALITYSD_ERROR_NULL_CALLBACK,
        TIMEQUALITYSD_ERROR_POOL_EXHAUSTED,
        TIMEQUALITYSD_ERROR_UNKNOWN_DESTROY,
        TIMEQUALITYSD_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource TimeQualitySdErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGTIMEQUALITYSDERRORS_H */
