#ifndef SOLIDSYSLOGSTRUCTUREDDATA_H
#define SOLIDSYSLOGSTRUCTUREDDATA_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    size_t SolidSyslogStructuredData_Format(struct SolidSyslogStructuredData * sd, char* buffer, size_t size);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTRUCTUREDDATA_H */
