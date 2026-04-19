#ifndef EXAMPLESWITCHCONFIG_H
#define EXAMPLESWITCHCONFIG_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    enum
    {
        EXAMPLE_SWITCH_UDP,
        EXAMPLE_SWITCH_TCP,
        EXAMPLE_SWITCH_COUNT,
    };

    void    ExampleSwitchConfig_SetByName(const char* name);
    uint8_t ExampleSwitchConfig_Selector(void); // NOLINT(modernize-redundant-void-arg) -- C idiom

EXTERN_C_END

#endif /* EXAMPLESWITCHCONFIG_H */
