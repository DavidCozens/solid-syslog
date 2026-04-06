#ifndef SOLIDSYSLOGORIGINSD_H
#define SOLIDSYSLOGORIGINSD_H

#include "ExternC.h"
#include "SolidSyslogAlloc.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(SolidSyslogAllocFunction alloc, const char* software, const char* swVersion);
    void                              SolidSyslogOriginSd_Destroy(struct SolidSyslogStructuredData * sd, SolidSyslogFreeFunction dealloc);

EXTERN_C_END

#endif /* SOLIDSYSLOGORIGINSD_H */
