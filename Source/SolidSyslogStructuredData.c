#include "SolidSyslogStructuredDataDef.h"

size_t SolidSyslogStructuredData_Format(struct SolidSyslogStructuredData* sd, char* buffer, size_t size)
{
    return sd->Format(sd, buffer, size);
}
