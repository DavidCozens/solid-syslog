/** @file
 *  POSIX file I/O (open / read / write / lseek / ftruncate) behind the
 *  SolidSyslogFile vtable, for a file-backed BlockDevice or Store. */
#ifndef SOLIDSYSLOGPOSIXFILE_H
#define SOLIDSYSLOGPOSIXFILE_H

#include "ExternC.h"

struct SolidSyslogFile;

EXTERN_C_BEGIN

    /** Create takes no config; an exhausted pool falls back to the shared
     *  NullFile. */
    struct SolidSyslogFile* SolidSyslogPosixFile_Create(void);
    /** Release the pool slot. */
    void SolidSyslogPosixFile_Destroy(struct SolidSyslogFile * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXFILE_H */
