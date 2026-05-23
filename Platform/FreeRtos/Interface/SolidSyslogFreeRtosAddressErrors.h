#ifndef SOLIDSYSLOGFREERTOSADDRESSERRORS_H
#define SOLIDSYSLOGFREERTOSADDRESSERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogFreeRtosAddressErrors
    {
        FREERTOSADDRESS_ERROR_POOL_EXHAUSTED,
        FREERTOSADDRESS_ERROR_UNKNOWN_DESTROY,
        FREERTOSADDRESS_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource FreeRtosAddressErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSADDRESSERRORS_H */
