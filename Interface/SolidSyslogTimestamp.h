#ifndef SOLIDSYSLOGTIMESTAMP_H
#define SOLIDSYSLOGTIMESTAMP_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogTimestamp
    {
        uint16_t year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  hour;
        uint8_t  minute;
        uint8_t  second;
        uint32_t microsecond;
        int16_t  utcOffsetMinutes;
    };

    typedef void (*SolidSyslogClockFunction)(struct SolidSyslogTimestamp* timestamp);

EXTERN_C_END

#endif /* SOLIDSYSLOGTIMESTAMP_H */
