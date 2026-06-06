#ifndef BDDTARGETLANGUAGE_H
#define BDDTARGETLANGUAGE_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogSdValue;

    void BddTargetLanguage_Get(struct SolidSyslogSdValue * value, void* context);

EXTERN_C_END

#endif /* BDDTARGETLANGUAGE_H */
