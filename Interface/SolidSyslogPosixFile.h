#ifndef SOLIDSYSLOGPOSIXFILE_H
#define SOLIDSYSLOGPOSIXFILE_H

#include "SolidSyslogFile.h"

#include <stdint.h>

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_POSIX_FILE_STORAGE_SLOTS = 11
    };

    struct SolidSyslogPosixFileStorage
    {
        uint8_t opaque[SOLIDSYSLOG_POSIX_FILE_STORAGE_SLOTS * sizeof(void*)];
    };

    struct SolidSyslogFile* SolidSyslogPosixFile_Create(struct SolidSyslogPosixFileStorage * storage);
    void                    SolidSyslogPosixFile_Destroy(struct SolidSyslogFile * file);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXFILE_H */
