#include "ExampleTcpConfig.h"
#include "SolidSyslogFormatter.h"

#include <stdint.h>
#include <string.h>

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

void ExampleTcpConfig_GetEndpoint(struct SolidSyslogEndpoint* endpoint)
{
    const char* host = ExampleTcpConfig_GetHost();
    SolidSyslogFormatter_BoundedString(endpoint->host, host, strlen(host));
    endpoint->port = (uint16_t) ExampleTcpConfig_GetPort();
}

/* Static config — host/port never change, so version stays 0 forever and the
   sender connects exactly once. */
uint32_t ExampleTcpConfig_GetEndpointVersion(void)
{
    return 0;
}
