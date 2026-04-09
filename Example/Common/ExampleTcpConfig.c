#include "ExampleTcpConfig.h"

enum
{
    EXAMPLE_TCP_PORT = 6514
};

const char* ExampleTcpConfig_GetHost(void)
{
    return "syslog-ng";
}

int ExampleTcpConfig_GetPort(void)
{
    return EXAMPLE_TCP_PORT;
}
