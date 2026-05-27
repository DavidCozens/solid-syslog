#ifndef SOLIDSYSLOGLWIPRAWTCPSTREAMPRIVATE_H
#define SOLIDSYSLOGLWIPRAWTCPSTREAMPRIVATE_H

#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogLwipRawTcpStream.h"
#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogTunables.h"

struct tcp_pcb;
struct pbuf;

struct SolidSyslogLwipRawTcpStream
{
    struct SolidSyslogStream                 Base;
    struct SolidSyslogLwipRawTcpStreamConfig Config;
    struct tcp_pcb*                          Pcb;
    bool                                     Connected;
    bool                                     Errored;
    struct pbuf*                             RxQueue[SOLIDSYSLOG_LWIP_RAW_TCP_RX_QUEUE_SIZE];
    size_t                                   RxQueueHead;
    size_t                                   RxQueueCount;
    size_t                                   RxHeadOffset;
};

void LwipRawTcpStream_Initialise(
    struct SolidSyslogStream* base,
    const struct SolidSyslogLwipRawTcpStreamConfig* config
);
void LwipRawTcpStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGLWIPRAWTCPSTREAMPRIVATE_H */
