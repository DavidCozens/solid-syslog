#ifndef SOLIDSYSLOGMETASD_H
#define SOLIDSYSLOGMETASD_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;
    struct SolidSyslogStructuredData;

    struct SolidSyslogStructuredData* SolidSyslogMetaSd_Create(struct SolidSyslogAtomicCounter * counter);
    void                              SolidSyslogMetaSd_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGMETASD_H */
