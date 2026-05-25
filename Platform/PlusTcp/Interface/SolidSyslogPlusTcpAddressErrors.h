#ifndef SOLIDSYSLOGPLUSTCPADDRESSERRORS_H
#define SOLIDSYSLOGPLUSTCPADDRESSERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogPlusTcpAddressErrors
    {
        PLUSTCPADDRESS_ERROR_POOL_EXHAUSTED,
        PLUSTCPADDRESS_ERROR_UNKNOWN_DESTROY,
        PLUSTCPADDRESS_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PlusTcpAddressErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPLUSTCPADDRESSERRORS_H */
