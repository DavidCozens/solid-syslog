#include "BddTargetSwitchConfig.h"

#include <string.h>

static volatile uint8_t selectedIndex;

void BddTargetSwitchConfig_SetByName(const char* name)
{
    if (strcmp(name, "udp") == 0)
    {
        selectedIndex = BDD_TARGET_SWITCH_UDP;
    }
    else if (strcmp(name, "tcp") == 0)
    {
        selectedIndex = BDD_TARGET_SWITCH_TCP;
    }
    else if ((strcmp(name, "tls") == 0) || (strcmp(name, "mtls") == 0))
    {
        /* mTLS shares the reliable/TLS slot; the TLS stream is configured
         * with client cert+key at startup when --transport mtls is selected. */
        selectedIndex = BDD_TARGET_SWITCH_TLS;
    }
}

uint8_t BddTargetSwitchConfig_Selector(void)
{
    return selectedIndex;
}
