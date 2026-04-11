#ifndef SOLIDSYSLOGFILESTORE_H
#define SOLIDSYSLOGFILESTORE_H

#include "SolidSyslogFile.h"
#include "SolidSyslogStore.h"

#include <stddef.h>

EXTERN_C_BEGIN

    enum SolidSyslogDiscardPolicy
    {
        SOLIDSYSLOG_DISCARD_OLDEST,
        SOLIDSYSLOG_DISCARD_NEWEST
    };

    struct SolidSyslogFileStoreConfig
    {
        struct SolidSyslogFile*       readFile;
        struct SolidSyslogFile*       writeFile;
        const char*                   pathPrefix;
        size_t                        maxFileSize;
        size_t                        maxFiles;
        enum SolidSyslogDiscardPolicy discardPolicy;
    };

    struct SolidSyslogStore* SolidSyslogFileStore_Create(const struct SolidSyslogFileStoreConfig* config);
    void                     SolidSyslogFileStore_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGFILESTORE_H */
