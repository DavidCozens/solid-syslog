#include "ExampleTcpConfig.h"

/* Unprivileged mirror of SOLIDSYSLOG_TCP_DEFAULT_PORT (514) for BDD containers */
enum
{
    EXAMPLE_TCP_PORT = 5514
};

const char* ExampleTcpConfig_GetHost(void)
{
    return "syslog-ng";
}

int ExampleTcpConfig_GetPort(void)
{
    return EXAMPLE_TCP_PORT;
}
