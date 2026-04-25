#ifndef SOLIDSYSLOGATOMICOPSDEFINITION_H
#define SOLIDSYSLOGATOMICOPSDEFINITION_H

#include "ExternC.h"

#include <stdbool.h>
#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogAtomicOps
    {
        uint32_t (*Load)(struct SolidSyslogAtomicOps* self);
        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) — CAS shape is universal (compare_exchange convention)
        bool (*CompareAndSwap)(struct SolidSyslogAtomicOps* self, uint32_t expected, uint32_t desired);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGATOMICOPSDEFINITION_H */
