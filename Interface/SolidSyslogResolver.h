#ifndef SOLIDSYSLOGRESOLVER_H
#define SOLIDSYSLOGRESOLVER_H

#include "ExternC.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogTransport.h"
#include <stdbool.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    bool SolidSyslogResolver_Resolve(struct SolidSyslogResolver * resolver, enum SolidSyslogTransport transport, struct SolidSyslogAddress * result);

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVER_H */
