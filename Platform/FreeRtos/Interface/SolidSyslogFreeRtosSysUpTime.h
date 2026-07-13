/** @file
 *  The FreeRTOS SolidSyslogSysUpTimeFunction, for the MetaSd structured-data
 *  element. */
#ifndef SOLIDSYSLOGFREERTOSSYSUPTIME_H
#define SOLIDSYSLOGFREERTOSSYSUPTIME_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /** Hundredths of a second derived from xTaskGetTickCount, as RFC 3418
     *  sysUpTime; a uint64 intermediate keeps the scaling correct at any
     *  configTICK_RATE_HZ. The value tracks the FreeRTOS tick counter, which
     *  itself wraps (well before 2^32 hundredths, and quickly under
     *  configUSE_16_BIT_TICKS), so uptime resets at that period. */
    uint32_t SolidSyslogFreeRtosSysUpTime_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSSYSUPTIME_H */
