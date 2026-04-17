#ifndef SOLIDSYSLOGTIMESTAMP_H
#define SOLIDSYSLOGTIMESTAMP_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /* Broken-down timestamp as emitted by a SolidSyslogClockFunction.
       All calendar/time fields are expressed in the timezone indicated by
       utcOffsetMinutes — the built-in POSIX and Windows clocks both produce
       UTC (utcOffsetMinutes == 0).

       A clock may leave the struct zeroed to signal "no usable timestamp";
       the formatter treats month == 0 as invalid. */
    struct SolidSyslogTimestamp
    {
        uint16_t year;             /* Gregorian year, e.g. 2026. 0 means invalid. */
        uint8_t  month;            /* 1-12. 0 means invalid. */
        uint8_t  day;              /* 1-31. */
        uint8_t  hour;             /* 0-23. */
        uint8_t  minute;           /* 0-59. */
        uint8_t  second;           /* 0-59. Leap seconds are not represented. */
        uint32_t microsecond;      /* 0-999999. */
        int16_t  utcOffsetMinutes; /* Offset from UTC in minutes; 0 for UTC. */
    };

    typedef void (*SolidSyslogClockFunction)(struct SolidSyslogTimestamp* timestamp);

EXTERN_C_END

#endif /* SOLIDSYSLOGTIMESTAMP_H */
