#include "ExampleSwitchConfig.h"

#include <string.h>

static volatile uint8_t selectedIndex;

void ExampleSwitchConfig_SetByName(const char* name)
{
    if (strcmp(name, "udp") == 0)
    {
        selectedIndex = EXAMPLE_SWITCH_UDP;
    }
    else if (strcmp(name, "tcp") == 0)
    {
        selectedIndex = EXAMPLE_SWITCH_TCP;
    }
    else if (strcmp(name, "tls") == 0)
    {
        selectedIndex = EXAMPLE_SWITCH_TLS;
    }
}

uint8_t ExampleSwitchConfig_Selector(void)
{
    return selectedIndex;
}
