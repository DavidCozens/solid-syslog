#include "SolidSyslogFatFsFile.h"

#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogFileDefinition.h"
#include "SolidSyslogMacros.h"
#include "ff.h"

#define READ_WRITE_OR_CREATE (FA_READ | FA_WRITE | FA_OPEN_ALWAYS)

static bool                                FatFsFile_Open(struct SolidSyslogFile* self, const char* path);
static void                                FatFsFile_Close(struct SolidSyslogFile* self);
static bool                                FatFsFile_IsOpen(struct SolidSyslogFile* self);
static inline struct SolidSyslogFatFsFile* Self(struct SolidSyslogFile* self);

struct SolidSyslogFatFsFile
{
    struct SolidSyslogFile base;
    FIL                    fp;
    bool                   isOpen;
};

SOLIDSYSLOG_STATIC_ASSERT(sizeof(struct SolidSyslogFatFsFile) <= sizeof(SolidSyslogFatFsFileStorage),
                          "SOLIDSYSLOG_FATFS_FILE_SIZE is too small for struct SolidSyslogFatFsFile");

static const struct SolidSyslogFatFsFile DEFAULT_INSTANCE = {
    .base   = {FatFsFile_Open, FatFsFile_Close, FatFsFile_IsOpen, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    .isOpen = false,
};

struct SolidSyslogFile* SolidSyslogFatFsFile_Create(SolidSyslogFatFsFileStorage* storage)
{
    struct SolidSyslogFatFsFile* fatfs = (struct SolidSyslogFatFsFile*) storage;
    *fatfs                             = DEFAULT_INSTANCE;
    return &fatfs->base;
}

void SolidSyslogFatFsFile_Destroy(struct SolidSyslogFile* file)
{
    FatFsFile_Close(file);
}

static bool FatFsFile_Open(struct SolidSyslogFile* self, const char* path)
{
    struct SolidSyslogFatFsFile* fatfs  = Self(self);
    FRESULT                      result = f_open(&fatfs->fp, path, READ_WRITE_OR_CREATE);
    fatfs->isOpen                       = (result == FR_OK);
    return fatfs->isOpen;
}

static inline struct SolidSyslogFatFsFile* Self(struct SolidSyslogFile* self)
{
    return (struct SolidSyslogFatFsFile*) self;
}

static void FatFsFile_Close(struct SolidSyslogFile* self)
{
    struct SolidSyslogFatFsFile* fatfs = Self(self);
    if (fatfs->isOpen)
    {
        f_close(&fatfs->fp);
        fatfs->isOpen = false;
    }
}

static bool FatFsFile_IsOpen(struct SolidSyslogFile* self)
{
    return Self(self)->isOpen;
}
