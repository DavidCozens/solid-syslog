#ifndef SOLIDSYSLOGPLUSTCPTCPSTREAMERRORS_H
#define SOLIDSYSLOGPLUSTCPTCPSTREAMERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogPlusTcpTcpStreamErrors
    {
        PLUSTCPTCPSTREAM_ERROR_POOL_EXHAUSTED,
        PLUSTCPTCPSTREAM_ERROR_UNKNOWN_DESTROY,
        PLUSTCPTCPSTREAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PlusTcpTcpStreamErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPLUSTCPTCPSTREAMERRORS_H */
