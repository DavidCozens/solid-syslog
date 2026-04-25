#ifndef SOLIDSYSLOGWINDOWSATOMICOPS_H
#define SOLIDSYSLOGWINDOWSATOMICOPS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAtomicOps;

    struct SolidSyslogAtomicOps* SolidSyslogWindowsAtomicOps_Create(void);
    void                         SolidSyslogWindowsAtomicOps_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGWINDOWSATOMICOPS_H */
