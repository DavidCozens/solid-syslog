#ifndef SOLIDSYSLOGLWIPRAWDNSRESOLVER_H
#define SOLIDSYSLOGLWIPRAWDNSRESOLVER_H

#include "ExternC.h"
#include "SolidSyslogSleep.h"

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    struct SolidSyslogLwipRawDnsResolverConfig
    {
        SolidSyslogSleepFunction
            Sleep; /* required — drives the bounded async-resolve spin; NULL config falls back to NullResolver */
    };

    struct SolidSyslogResolver* SolidSyslogLwipRawDnsResolver_Create(
        const struct SolidSyslogLwipRawDnsResolverConfig* config
    );
    void SolidSyslogLwipRawDnsResolver_Destroy(struct SolidSyslogResolver * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGLWIPRAWDNSRESOLVER_H */
