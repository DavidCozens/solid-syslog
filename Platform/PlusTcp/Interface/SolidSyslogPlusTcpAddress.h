#ifndef SOLIDSYSLOGPLUSTCPADDRESS_H
#define SOLIDSYSLOGPLUSTCPADDRESS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogAddress;

    struct SolidSyslogAddress* SolidSyslogPlusTcpAddress_Create(void);
    void SolidSyslogPlusTcpAddress_Destroy(struct SolidSyslogAddress * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGPLUSTCPADDRESS_H */
