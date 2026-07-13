/** @file
 *  The POSIX SolidSyslogSysUpTimeFunction, for MetaSd. */
#ifndef SOLIDSYSLOGPOSIXSYSUPTIME_H
#define SOLIDSYSLOGPOSIXSYSUPTIME_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /** Hundredths of a second since boot from CLOCK_BOOTTIME, as RFC 3418
     *  sysUpTime; wraps modulo 2^32 per the TimeTicks contract. */
    uint32_t SolidSyslogPosixSysUpTime_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXSYSUPTIME_H */
