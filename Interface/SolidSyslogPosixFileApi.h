#ifndef SOLIDSYSLOGPOSIXFILEAPI_H
#define SOLIDSYSLOGPOSIXFILEAPI_H

#include "SolidSyslogFileApi.h"

EXTERN_C_BEGIN

    struct SolidSyslogFileApi* SolidSyslogPosixFileApi_Create(void);
    void                       SolidSyslogPosixFileApi_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXFILEAPI_H */
