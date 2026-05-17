#ifndef SOLIDSYSLOGATOMICCOUNTER_H
#define SOLIDSYSLOGATOMICCOUNTER_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;

    uint32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGATOMICCOUNTER_H */
