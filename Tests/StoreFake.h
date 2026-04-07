#ifndef STOREFAKE_H
#define STOREFAKE_H

#include "SolidSyslogStore.h"

EXTERN_C_BEGIN

    struct SolidSyslogStore* StoreFake_Create(void);
    void                     StoreFake_Destroy(void);
    void                     StoreFake_FailNextWrite(void);

EXTERN_C_END

#endif /* STOREFAKE_H */
