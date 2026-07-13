/** @file
 *  The POSIX SolidSyslogSleepFunction. */
#ifndef SOLIDSYSLOGPOSIXSLEEP_H
#define SOLIDSYSLOGPOSIXSLEEP_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Wraps nanosleep so a bounded retry loop (e.g. the TLS handshake) yields to
     *  the kernel between attempts. */
    void SolidSyslogPosixSleep(int milliseconds);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXSLEEP_H */
