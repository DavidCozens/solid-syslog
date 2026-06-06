#ifndef SOLIDSYSLOGMETASD_H
#define SOLIDSYSLOGMETASD_H

#include "ExternC.h"
#include "SolidSyslogSdValueFunction.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;
    struct SolidSyslogStructuredData;

    typedef uint32_t (*SolidSyslogSysUpTimeFunction)(void);

    struct SolidSyslogMetaSdConfig
    {
        struct SolidSyslogAtomicCounter* Counter;
        SolidSyslogSysUpTimeFunction GetSysUpTime;
        SolidSyslogSdValueFunction GetLanguage;
        void* LanguageContext;
    };

    struct SolidSyslogStructuredData* SolidSyslogMetaSd_Create(const struct SolidSyslogMetaSdConfig* config);
    void SolidSyslogMetaSd_Destroy(struct SolidSyslogStructuredData * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGMETASD_H */
