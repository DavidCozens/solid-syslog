#ifndef SOLIDSYSLOGSTDATOMICCOUNTER_H
#define SOLIDSYSLOGSTDATOMICCOUNTER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;

    struct SolidSyslogAtomicCounter* SolidSyslogStdAtomicCounter_Create(void);
    void SolidSyslogStdAtomicCounter_Destroy(struct SolidSyslogAtomicCounter * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTDATOMICCOUNTER_H */
