#ifndef SOLIDSYSLOGSTDATOMICOPS_H
#define SOLIDSYSLOGSTDATOMICOPS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAtomicOps;

    struct SolidSyslogAtomicOps* SolidSyslogStdAtomicOps_Create(void);
    void                         SolidSyslogStdAtomicOps_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTDATOMICOPS_H */
