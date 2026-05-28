#ifndef SOLIDSYSLOGLWIPRAWMARSHAL_H
#define SOLIDSYSLOGLWIPRAWMARSHAL_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /* Every lwIP Raw API call the SolidSyslog wrappers make is routed through
       a single marshal hop so an integrator can pin those calls to the thread
       that owns the lwIP core.

       NO_SYS=1 (bare metal, no RTOS): the default direct-call marshal is
       correct — there is one execution context and no core to protect.

       NO_SYS=0 (RTOS with a tcpip thread): the integrator installs a marshal
       that hops onto the tcpip thread — e.g. tcpip_callback_with_block, or a
       LOCK_TCPIP_CORE / UNLOCK_TCPIP_CORE pair.

       Contract: the marshal MUST invoke its callback synchronously — before
       the marshal function returns. The wrapper reads results the callback
       writes immediately after the hop, so an asynchronous marshal is caller
       error. tcpip_callback_with_block(.., block=1) honours this; a bare
       tcpip_callback(..) does not. */
    typedef void (*SolidSyslogLwipRawCallback)(void* context);
    typedef void (*SolidSyslogLwipRawMarshalFunction)(SolidSyslogLwipRawCallback callback, void* context);

    /* Installs a process-global marshal. There is one lwIP instance and one
       tcpip thread per process, so a single global slot suffices — same shape
       as SolidSyslog_SetErrorHandler. Passing NULL restores the direct-call
       default. Intended for setup-time configuration; not synchronised with
       concurrent installs. */
    void SolidSyslogLwipRaw_SetMarshal(SolidSyslogLwipRawMarshalFunction marshal);

EXTERN_C_END

#endif /* SOLIDSYSLOGLWIPRAWMARSHAL_H */
