#include "ExampleUdpConfig.h"

enum
{
    EXAMPLE_UDP_PORT = 5514
};

const char* ExampleUdpConfig_GetHost(void)
{
    return "syslog-ng";
}

int ExampleUdpConfig_GetPort(void)
{
    return EXAMPLE_UDP_PORT;
}
