#ifndef SOLIDSYSLOGPOSIXCLOCK_H
#define SOLIDSYSLOGPOSIXCLOCK_H

#include "SolidSyslogTimestamp.h"

EXTERN_C_BEGIN

    struct SolidSyslogTimestamp SolidSyslogPosixClock_GetTimestamp(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXCLOCK_H */
