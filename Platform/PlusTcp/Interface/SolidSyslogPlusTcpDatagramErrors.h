#ifndef SOLIDSYSLOGPLUSTCPDATAGRAMERRORS_H
#define SOLIDSYSLOGPLUSTCPDATAGRAMERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogPlusTcpDatagramErrors
    {
        PLUSTCPDATAGRAM_ERROR_POOL_EXHAUSTED,
        PLUSTCPDATAGRAM_ERROR_UNKNOWN_DESTROY,
        PLUSTCPDATAGRAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PlusTcpDatagramErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPLUSTCPDATAGRAMERRORS_H */
