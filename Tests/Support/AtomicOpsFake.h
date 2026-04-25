#ifndef ATOMICOPSFAKE_H
#define ATOMICOPSFAKE_H

#include "ExternC.h"
#include "SolidSyslogAtomicOpsDefinition.h"

#include <stdint.h>

EXTERN_C_BEGIN

    void                         AtomicOpsFake_Reset(void);
    struct SolidSyslogAtomicOps* AtomicOpsFake_Get(void);
    void                         AtomicOpsFake_SetLoadValue(uint32_t value);
    void                         AtomicOpsFake_FailNextCompareAndSwapAndShiftLoadTo(uint32_t value);

EXTERN_C_END

#endif /* ATOMICOPSFAKE_H */
