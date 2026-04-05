#ifndef SOLIDSYSLOGSTRUCTUREDDATADEF_H
#define SOLIDSYSLOGSTRUCTUREDDATADEF_H

#include "SolidSyslogStructuredData.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData
    {
        size_t (*Format)(struct SolidSyslogStructuredData* self, char* buffer, size_t size);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSTRUCTUREDDATADEF_H */
