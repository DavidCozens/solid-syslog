#ifndef SOLIDSYSLOGATOMICCOUNTER_H
#define SOLIDSYSLOGATOMICCOUNTER_H

#include "ExternC.h"
#include "SolidSyslogAlloc.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;

    struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(SolidSyslogAllocFunction alloc);
    void                             SolidSyslogAtomicCounter_Destroy(struct SolidSyslogAtomicCounter * counter, SolidSyslogFreeFunction dealloc);
    uint_fast32_t                    SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter * counter);

EXTERN_C_END

#endif /* SOLIDSYSLOGATOMICCOUNTER_H */
