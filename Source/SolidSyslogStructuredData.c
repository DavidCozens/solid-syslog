#include "SolidSyslogStructuredDataDefinition.h"

void SolidSyslogStructuredData_Format(struct SolidSyslogStructuredData* sd, struct SolidSyslogFormatter* formatter)
{
    sd->Format(sd, formatter);
}
