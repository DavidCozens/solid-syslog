#ifndef SOLIDSYSLOGFATFSFILE_H
#define SOLIDSYSLOGFATFSFILE_H

#include "ExternC.h"

struct SolidSyslogFile;

EXTERN_C_BEGIN

    struct SolidSyslogFile* SolidSyslogFatFsFile_Create(void);
    void SolidSyslogFatFsFile_Destroy(struct SolidSyslogFile * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGFATFSFILE_H */
