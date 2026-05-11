#ifndef BDDTARGETSWITCHCONFIG_H
#define BDDTARGETSWITCHCONFIG_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    enum
    {
        BDD_TARGET_SWITCH_UDP,
        BDD_TARGET_SWITCH_TCP,
        BDD_TARGET_SWITCH_TLS,
        BDD_TARGET_SWITCH_COUNT,
    };

    void    BddTargetSwitchConfig_SetByName(const char* name);
    uint8_t BddTargetSwitchConfig_Selector(void); // NOLINT(modernize-redundant-void-arg) -- C idiom

EXTERN_C_END

#endif /* BDDTARGETSWITCHCONFIG_H */
