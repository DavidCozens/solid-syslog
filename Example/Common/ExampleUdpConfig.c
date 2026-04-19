#include "ExampleUdpConfig.h"
#include "SolidSyslogFormatter.h"

#include <stdint.h>
#include <string.h>

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

void ExampleUdpConfig_GetEndpoint(struct SolidSyslogEndpoint* endpoint)
{
    const char* host = ExampleUdpConfig_GetHost();
    SolidSyslogFormatter_BoundedString(endpoint->host, host, strlen(host));
    endpoint->port    = (uint16_t) ExampleUdpConfig_GetPort();
    endpoint->version = 0;
}
