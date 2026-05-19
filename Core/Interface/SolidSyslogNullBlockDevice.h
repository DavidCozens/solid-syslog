#ifndef SOLIDSYSLOGNULLBLOCKDEVICE_H
#define SOLIDSYSLOGNULLBLOCKDEVICE_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogBlockDevice;

    struct SolidSyslogBlockDevice* SolidSyslogNullBlockDevice_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLBLOCKDEVICE_H */
