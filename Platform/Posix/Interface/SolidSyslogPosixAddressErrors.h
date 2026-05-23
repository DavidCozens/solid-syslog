#ifndef SOLIDSYSLOGPOSIXADDRESSERRORS_H
#define SOLIDSYSLOGPOSIXADDRESSERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogPosixAddressErrors
    {
        POSIXADDRESS_ERROR_POOL_EXHAUSTED,
        POSIXADDRESS_ERROR_UNKNOWN_DESTROY,
        POSIXADDRESS_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PosixAddressErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXADDRESSERRORS_H */
