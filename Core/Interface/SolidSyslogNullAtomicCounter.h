#ifndef SOLIDSYSLOGNULLATOMICCOUNTER_H
#define SOLIDSYSLOGNULLATOMICCOUNTER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Increment returns 1U unconditionally. RFC 5424 §7.3.1 forbids a sequenceId of 0,
     *  and 1U is indistinguishable from the post-power-on or post-wrap state, so it is the
     *  safest value when a real counter is unavailable. */
    struct SolidSyslogAtomicCounter* SolidSyslogNullAtomicCounter_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLATOMICCOUNTER_H */
