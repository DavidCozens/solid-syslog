#ifndef SOLIDSYSLOGFATFSFILEPRIVATE_H
#define SOLIDSYSLOGFATFSFILEPRIVATE_H

#include <stdbool.h>

#include "SolidSyslogFileDefinition.h"
#include "ff.h"

struct SolidSyslogFatFsFile
{
    struct SolidSyslogFile Base;
    FIL Fp;
    bool IsOpen;
};

void FatFsFile_Initialise(struct SolidSyslogFile* base);
void FatFsFile_Cleanup(struct SolidSyslogFile* base);

#endif /* SOLIDSYSLOGFATFSFILEPRIVATE_H */
