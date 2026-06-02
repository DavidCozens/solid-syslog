#ifndef SOLIDSYSLOGSECURITYPOLICYDEFINITION_H
#define SOLIDSYSLOGSECURITYPOLICYDEFINITION_H

#include "ExternC.h"

#include <stdbool.h>
#include <stdint.h>

EXTERN_C_BEGIN

    /* One record presented to a security policy. The Content span is split as
     *   Content[0 .. HeaderLength)             associated data — authenticated,
     *                                          never encrypted (the cleartext
     *                                          header the reader needs intact)
     *   Content[HeaderLength .. ContentLength) body — authenticated; an AEAD
     *                                          policy also encrypts it in place
     * and Trailer is the policy-owned span of TrailerSize bytes. MAC and
     * checksum policies authenticate the whole Content and ignore the split;
     * AEAD policies treat the header as associated data and encrypt the body.
     *
     * Passed by const pointer: a policy may not repoint the fields, but the
     * member pointers are non-const so it can write through Content (in-place
     * encrypt/decrypt) and Trailer (seal) per the contract. */
    struct SolidSyslogSecurityRecord
    {
        uint8_t* Content;
        uint16_t ContentLength;
        uint16_t HeaderLength;
        uint8_t* Trailer; /* written by SealRecord, read by OpenRecord */
    };

    struct SolidSyslogSecurityPolicy
    {
        uint16_t TrailerSize;
        bool (*SealRecord)(struct SolidSyslogSecurityPolicy* self, const struct SolidSyslogSecurityRecord* record);
        bool (*OpenRecord)(struct SolidSyslogSecurityPolicy* self, const struct SolidSyslogSecurityRecord* record);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSECURITYPOLICYDEFINITION_H */
