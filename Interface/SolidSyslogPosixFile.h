#ifndef SOLIDSYSLOGPOSIXFILE_H
#define SOLIDSYSLOGPOSIXFILE_H

#include "SolidSyslogFile.h"

EXTERN_C_BEGIN

    struct SolidSyslogFile* SolidSyslogPosixFile_Create(void);
    void                    SolidSyslogPosixFile_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXFILE_H */
