#ifndef SOLIDSYSLOGSTRUCTUREDDATA_H
#define SOLIDSYSLOGSTRUCTUREDDATA_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogFormatter;
    struct SolidSyslogStructuredData;

    void SolidSyslogStructuredData_Format(struct SolidSyslogStructuredData * sd, struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* SOLIDSYSLOGSTRUCTUREDDATA_H */
