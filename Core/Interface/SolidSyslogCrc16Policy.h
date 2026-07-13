#ifndef SOLIDSYSLOGCRC16POLICY_H
#define SOLIDSYSLOGCRC16POLICY_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** An unkeyed at-rest integrity policy: it appends a CRC-16 trailer that
     *  catches accidental corruption of a stored record, but offers no defence
     *  against a deliberate edit (a tamperer can recompute the CRC). For
     *  tamper-evidence or confidentiality use a keyed policy (HmacSha256,
     *  AesGcm) instead. Returns a shared stateless instance; never NULL. */
    struct SolidSyslogSecurityPolicy* SolidSyslogCrc16Policy_Create(void);
    /** No-op: the policy is stateless and holds no pool slot. Present for
     *  lifecycle symmetry with the keyed policies. */
    void SolidSyslogCrc16Policy_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGCRC16POLICY_H */
