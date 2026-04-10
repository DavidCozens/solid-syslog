#ifndef FILEFAKE_H
#define FILEFAKE_H

#include "SolidSyslogFileApi.h"

EXTERN_C_BEGIN

    struct SolidSyslogFileApi* FileFake_Create(void);
    void                       FileFake_Destroy(void);
    void                       FileFake_FailNextOpen(void);
    void                       FileFake_FailNextWrite(void);
    void                       FileFake_FailNextRead(void);
    const void*                FileFake_FileContent(void);
    size_t                     FileFake_FileSize(void);

EXTERN_C_END

#endif /* FILEFAKE_H */
