/** @file
 *  The FreeRTOS SolidSyslogSysUpTimeFunction, for MetaSd. */
#ifndef SOLIDSYSLOGFREERTOSSYSUPTIME_H
#define SOLIDSYSLOGFREERTOSSYSUPTIME_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /** Hundredths of a second since boot from xTaskGetTickCount, as RFC 3418
     *  sysUpTime; a uint64 intermediate keeps the result correct at any
     *  configTICK_RATE_HZ, then wraps modulo 2^32 per the TimeTicks contract. */
    uint32_t SolidSyslogFreeRtosSysUpTime_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSSYSUPTIME_H */
