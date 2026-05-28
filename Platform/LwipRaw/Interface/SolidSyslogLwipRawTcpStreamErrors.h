#ifndef SOLIDSYSLOGLWIPRAWTCPSTREAMERRORS_H
#define SOLIDSYSLOGLWIPRAWTCPSTREAMERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogLwipRawTcpStreamErrors
    {
        LWIPRAWTCPSTREAM_ERROR_POOL_EXHAUSTED,
        LWIPRAWTCPSTREAM_ERROR_UNKNOWN_DESTROY,
        LWIPRAWTCPSTREAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource LwipRawTcpStreamErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGLWIPRAWTCPSTREAMERRORS_H */
