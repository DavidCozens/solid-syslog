#ifndef SOLIDSYSLOGWINSOCKDATAGRAMERRORS_H
#define SOLIDSYSLOGWINSOCKDATAGRAMERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogWinsockDatagramErrors
    {
        WINSOCKDATAGRAM_ERROR_POOL_EXHAUSTED,
        WINSOCKDATAGRAM_ERROR_UNKNOWN_DESTROY,
        WINSOCKDATAGRAM_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WinsockDatagramErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINSOCKDATAGRAMERRORS_H */
