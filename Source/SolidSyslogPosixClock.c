#include "SolidSyslogPosixClock.h"

#include <time.h>

struct SolidSyslogTimestamp SolidSyslogPosixClock_GetTimestamp(void)
{
    struct SolidSyslogTimestamp timestamp = {0};
    struct timespec             now;
    struct tm                   breakdown;

    if (clock_gettime(CLOCK_REALTIME, &now) == 0 && gmtime_r(&now.tv_sec, &breakdown) != NULL)
    {
        timestamp.year        = (uint16_t)(breakdown.tm_year + 1900);
        timestamp.month       = (uint8_t)(breakdown.tm_mon + 1);
        timestamp.day         = (uint8_t)breakdown.tm_mday;
        timestamp.hour        = (uint8_t)breakdown.tm_hour;
        timestamp.minute      = (uint8_t)breakdown.tm_min;
        timestamp.second      = (uint8_t)breakdown.tm_sec;
        timestamp.microsecond = (uint32_t)(now.tv_nsec / 1000);
    }

    return timestamp;
}
