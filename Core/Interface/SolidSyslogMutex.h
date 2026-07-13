#ifndef SOLIDSYSLOGMUTEX_H
#define SOLIDSYSLOGMUTEX_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogMutex;

    /** Dispatch to the injected mutex's vtable; behaviour, including whether Lock
     *  blocks, is that mutex's (see SolidSyslogMutexDefinition.h). */
    void SolidSyslogMutex_Lock(struct SolidSyslogMutex * mutex);
    void SolidSyslogMutex_Unlock(struct SolidSyslogMutex * mutex);

EXTERN_C_END

#endif /* SOLIDSYSLOGMUTEX_H */
