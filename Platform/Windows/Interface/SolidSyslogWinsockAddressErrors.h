#ifndef SOLIDSYSLOGWINSOCKADDRESSERRORS_H
#define SOLIDSYSLOGWINSOCKADDRESSERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogWinsockAddressErrors
    {
        WINSOCKADDRESS_ERROR_POOL_EXHAUSTED,
        WINSOCKADDRESS_ERROR_UNKNOWN_DESTROY,
        WINSOCKADDRESS_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WinsockAddressErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINSOCKADDRESSERRORS_H */
