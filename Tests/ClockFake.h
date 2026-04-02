#ifndef CLOCKFAKE_H
#define CLOCKFAKE_H

#include "ExternC.h"

#include <time.h>

EXTERN_C_BEGIN

    void ClockFake_Reset(void);
    void ClockFake_SetTime(time_t seconds, long nanoseconds);
    void ClockFake_SetClockGettimeReturn(int returnValue);
    void ClockFake_SetGmtimeReturn(struct tm* returnValue);

EXTERN_C_END

#endif /* CLOCKFAKE_H */
