#ifndef SOLIDSYSLOGNULLFILE_H
#define SOLIDSYSLOGNULLFILE_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Open, IsOpen, Read and Exists return false, so consumers see a consistently
     *  non-functional file and take their error path. Write and Delete return true,
     *  reporting success vacuously so callers that treat those as no-ops are not tripped.
     *  SeekTo, Truncate and Close are no-ops; Size returns 0. */
    struct SolidSyslogFile* SolidSyslogNullFile_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLFILE_H */
