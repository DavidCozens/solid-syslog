#ifndef SOLIDSYSLOGORIGINSD_H
#define SOLIDSYSLOGORIGINSD_H

#include "ExternC.h"

#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogSdValue;
    struct SolidSyslogStructuredData;

    typedef size_t (*SolidSyslogOriginIpCountFunction)(void);
    typedef void (*SolidSyslogOriginIpAtFunction)(struct SolidSyslogSdValue* value, void* context, size_t index);

    struct SolidSyslogOriginSdConfig
    {
        const char* Software;
        const char* SwVersion;
        const char* EnterpriseId;
        SolidSyslogOriginIpCountFunction GetIpCount;
        SolidSyslogOriginIpAtFunction GetIpAt;
        void* IpContext;
    };

    struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(const struct SolidSyslogOriginSdConfig* config);
    void SolidSyslogOriginSd_Destroy(struct SolidSyslogStructuredData * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGORIGINSD_H */
