#ifndef SOLIDSYSLOGNULLRESOLVER_H
#define SOLIDSYSLOGNULLRESOLVER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Resolve returns false (could not resolve), so the caller's existing
     *  unresolved-host error path runs naturally. */
    struct SolidSyslogResolver* SolidSyslogNullResolver_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLRESOLVER_H */
