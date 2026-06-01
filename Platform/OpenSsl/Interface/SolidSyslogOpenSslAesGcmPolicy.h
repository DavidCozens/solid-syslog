#ifndef SOLIDSYSLOGOPENSSLAESGCMPOLICY_H
#define SOLIDSYSLOGOPENSSLAESGCMPOLICY_H

#include "ExternC.h"
#include "SolidSyslogKeyFunction.h"

struct SolidSyslogSecurityPolicy;

EXTERN_C_BEGIN

    struct SolidSyslogOpenSslAesGcmPolicyConfig
    {
        SolidSyslogKeyFunction GetKey; /* fetches the 32-byte AES-256 key on demand — required */
        void* KeyContext; /* passed through to GetKey; NULL is fine */
    };

    struct SolidSyslogSecurityPolicy* SolidSyslogOpenSslAesGcmPolicy_Create(
        const struct SolidSyslogOpenSslAesGcmPolicyConfig* config
    );
    void SolidSyslogOpenSslAesGcmPolicy_Destroy(struct SolidSyslogSecurityPolicy * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGOPENSSLAESGCMPOLICY_H */
