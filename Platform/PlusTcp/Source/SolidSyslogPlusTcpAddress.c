#include "SolidSyslogPlusTcpAddressPrivate.h"

#include <string.h>

struct SolidSyslogAddress;

void PlusTcpAddress_Initialise(struct SolidSyslogAddress* base)
{
    struct SolidSyslogPlusTcpAddress* self = (struct SolidSyslogPlusTcpAddress*) base;
    (void) memset(&self->Sockaddr, 0, sizeof(self->Sockaddr));
}

void PlusTcpAddress_Cleanup(struct SolidSyslogAddress* base)
{
    (void) base;
}
