#ifndef BDDTARGETTLSSENDER_H
#define BDDTARGETTLSSENDER_H

#include "ExternC.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;
    struct SolidSyslogSender;

    struct SolidSyslogSender* BddTargetTlsSender_Create(struct SolidSyslogResolver * resolver, bool mtls);
    void                      BddTargetTlsSender_Destroy(void);

EXTERN_C_END

#endif /* BDDTARGETTLSSENDER_H */
