#ifndef BDDTARGETAPPNAME_H
#define BDDTARGETAPPNAME_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogFormatter;

    void BddTargetAppName_Set(const char* argv0);
    void BddTargetAppName_Get(struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* BDDTARGETAPPNAME_H */
