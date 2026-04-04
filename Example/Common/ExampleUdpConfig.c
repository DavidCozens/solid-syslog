#include "ExampleUdpConfig.h"

const char* ExampleUdpConfig_GetHost(void)
{
    return "syslog-ng";
}

int ExampleUdpConfig_GetPort(void)
{
    return 5514;
}
