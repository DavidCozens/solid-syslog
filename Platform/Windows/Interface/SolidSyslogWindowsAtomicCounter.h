#ifndef SOLIDSYSLOGWINDOWSATOMICCOUNTER_H
#define SOLIDSYSLOGWINDOWSATOMICCOUNTER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;

    struct SolidSyslogAtomicCounter* SolidSyslogWindowsAtomicCounter_Create(void);
    void SolidSyslogWindowsAtomicCounter_Destroy(struct SolidSyslogAtomicCounter * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGWINDOWSATOMICCOUNTER_H */
