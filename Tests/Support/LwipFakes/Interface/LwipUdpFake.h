#ifndef LWIPUDPFAKE_H
#define LWIPUDPFAKE_H

#include "ExternC.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    struct udp_pcb;

    void LwipUdpFake_Reset(void);

    /* udp_new configuration */
    void LwipUdpFake_SetUdpNewFails(bool fails);

    /* udp_new spy */
    unsigned LwipUdpFake_UdpNewCallCount(void);
    struct udp_pcb* LwipUdpFake_LastUdpNewReturned(void);

    /* udp_remove spy */
    unsigned LwipUdpFake_UdpRemoveCallCount(void);
    struct udp_pcb* LwipUdpFake_LastUdpRemovePcb(void);

EXTERN_C_END

#endif /* LWIPUDPFAKE_H */
