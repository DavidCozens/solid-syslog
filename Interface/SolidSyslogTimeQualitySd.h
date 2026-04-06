#ifndef SOLIDSYSLOGTIMEQUALITYSD_H
#define SOLIDSYSLOGTIMEQUALITYSD_H

#include "ExternC.h"
#include "SolidSyslogAlloc.h"
#include "SolidSyslogTimeQuality.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    struct SolidSyslogStructuredData* SolidSyslogTimeQualitySd_Create(SolidSyslogAllocFunction alloc, SolidSyslogTimeQualityFunction getTimeQuality);
    void                              SolidSyslogTimeQualitySd_Destroy(struct SolidSyslogStructuredData * sd, SolidSyslogFreeFunction dealloc);

EXTERN_C_END

#endif /* SOLIDSYSLOGTIMEQUALITYSD_H */
