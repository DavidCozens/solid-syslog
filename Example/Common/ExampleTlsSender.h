#ifndef EXAMPLETLSSENDER_H
#define EXAMPLETLSSENDER_H

#include "ExternC.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;
    struct SolidSyslogSender;

    struct SolidSyslogSender* ExampleTlsSender_Create(struct SolidSyslogResolver * resolver, bool mtls);
    void                      ExampleTlsSender_Destroy(void);

EXTERN_C_END

#endif /* EXAMPLETLSSENDER_H */
