#include "ExampleUdpConfig.h"

/* Unprivileged mirror of SOLIDSYSLOG_UDP_DEFAULT_PORT (514) for BDD containers */
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
