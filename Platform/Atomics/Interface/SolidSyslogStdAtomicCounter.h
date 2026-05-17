#ifndef SOLIDSYSLOGSTDATOMICCOUNTER_H
#define SOLIDSYSLOGSTDATOMICCOUNTER_H

#include "ExternC.h"
#include "SolidSyslogAtomicCounter.h"

#include <stdint.h>

EXTERN_C_BEGIN

    typedef struct
    {
        intptr_t Slot;
    } SolidSyslogStdAtomicCounterStorage;

    struct SolidSyslogAtomicCounter * SolidSyslogStdAtomicCounter_Create(SolidSyslogStdAtomicCounterStorage * storage);
    uint32_t SolidSyslogStdAtomicCounter_Increment(struct SolidSyslogAtomicCounter * self);
    void SolidSyslogStdAtomicCounter_Destroy(struct SolidSyslogAtomicCounter * self);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTDATOMICCOUNTER_H */
