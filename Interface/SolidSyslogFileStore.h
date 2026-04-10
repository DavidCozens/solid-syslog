#ifndef SOLIDSYSLOGFILESTORE_H
#define SOLIDSYSLOGFILESTORE_H

#include "SolidSyslogFileApi.h"
#include "SolidSyslogStore.h"

EXTERN_C_BEGIN

    struct SolidSyslogStore* SolidSyslogFileStore_Create(struct SolidSyslogFileApi * fileApi, const char* path);
    void                     SolidSyslogFileStore_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGFILESTORE_H */
