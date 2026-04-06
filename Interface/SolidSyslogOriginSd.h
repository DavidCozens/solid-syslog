#ifndef SOLIDSYSLOGORIGINSD_H
#define SOLIDSYSLOGORIGINSD_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(const char* software, const char* swVersion);
    void                              SolidSyslogOriginSd_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGORIGINSD_H */
