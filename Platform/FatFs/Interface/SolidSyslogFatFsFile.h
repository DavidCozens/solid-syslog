#ifndef SOLIDSYSLOGFATFSFILE_H
#define SOLIDSYSLOGFATFSFILE_H

#include <stdint.h>

#include "ExternC.h"

struct SolidSyslogFile;

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_FATFS_FILE_SIZE = sizeof(intptr_t) * 90
    };

    typedef struct
    {
        intptr_t slots[(SOLIDSYSLOG_FATFS_FILE_SIZE + sizeof(intptr_t) - 1) / sizeof(intptr_t)];
    } SolidSyslogFatFsFileStorage;

    struct SolidSyslogFile* SolidSyslogFatFsFile_Create(SolidSyslogFatFsFileStorage * storage);
    void                    SolidSyslogFatFsFile_Destroy(struct SolidSyslogFile * file);

EXTERN_C_END

#endif /* SOLIDSYSLOGFATFSFILE_H */
