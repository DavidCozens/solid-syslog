#ifndef SOLIDSYSLOGNULLMUTEX_H
#define SOLIDSYSLOGNULLMUTEX_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Lock and Unlock are no-ops, giving unsynchronised access for single-task
     *  targets that need no mutual exclusion. */
    struct SolidSyslogMutex* SolidSyslogNullMutex_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLMUTEX_H */
