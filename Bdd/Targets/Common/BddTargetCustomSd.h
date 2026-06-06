#ifndef BDDTARGETCUSTOMSD_H
#define BDDTARGETCUSTOMSD_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogStructuredData;

    /* The worked custom SD-ELEMENT for the integrator guide
     * (docs/structured-data.md). Emits [example@32473 detail="..."]. The detail
     * value deliberately contains ", \ and ] so the oracle round-trip BDD can
     * prove the library applies RFC 5424 PARAM-VALUE escaping. Stateless
     * singleton — handed to SolidSyslog_LogWithSd by the `send-custom` command. */
    struct SolidSyslogStructuredData* BddTargetCustomSd_Get(void);

EXTERN_C_END

#endif /* BDDTARGETCUSTOMSD_H */
