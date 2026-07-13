/** @file
 *  The POSIX SolidSyslogClockFunction, for SolidSyslogConfig.Clock. */
#ifndef SOLIDSYSLOGPOSIXCLOCK_H
#define SOLIDSYSLOGPOSIXCLOCK_H

#include "ExternC.h"

struct SolidSyslogTimestamp;

EXTERN_C_BEGIN

    /** Fills @p timestamp from the system real-time clock (CLOCK_REALTIME). */
    void SolidSyslogPosixClock_GetTimestamp(struct SolidSyslogTimestamp * timestamp);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXCLOCK_H */
