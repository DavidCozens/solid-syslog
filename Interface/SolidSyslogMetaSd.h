#ifndef SOLIDSYSLOGMETASD_H
#define SOLIDSYSLOGMETASD_H

#include "ExternC.h"
#include "SolidSyslogAlloc.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    struct SolidSyslogStructuredData* SolidSyslogMetaSd_Create(SolidSyslogAllocFunction alloc);
    void                              SolidSyslogMetaSd_Destroy(struct SolidSyslogStructuredData* sd, SolidSyslogFreeFunction dealloc);

EXTERN_C_END

#endif /* SOLIDSYSLOGMETASD_H */
