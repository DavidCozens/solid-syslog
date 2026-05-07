#ifndef SOLIDSYSLOGTLSSTREAMINTERNAL_H
#define SOLIDSYSLOGTLSSTREAMINTERNAL_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /* Function-pointer seam for the TLS handshake retry loop. Production wires
       this to a platform-conditional sleep (Sleep on Windows, nanosleep on
       POSIX); tests UT_PTR_SET it to a no-op so the bounded retry budget
       drains in microseconds rather than its real wall-clock duration. */
    typedef void (*TlsStreamSleepFn)(int milliseconds);
    extern TlsStreamSleepFn TlsStream_sleep;

EXTERN_C_END

#endif /* SOLIDSYSLOGTLSSTREAMINTERNAL_H */
