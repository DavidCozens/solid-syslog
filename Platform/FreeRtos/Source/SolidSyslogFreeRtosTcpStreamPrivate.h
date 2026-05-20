#ifndef SOLIDSYSLOGFREERTOSTCPSTREAMPRIVATE_H
#define SOLIDSYSLOGFREERTOSTCPSTREAMPRIVATE_H

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

#include "SolidSyslogStreamDefinition.h"

struct SolidSyslogFreeRtosTcpStream
{
    struct SolidSyslogStream Base;
    Socket_t Socket;
};

void FreeRtosTcpStream_Initialise(struct SolidSyslogStream* base);
void FreeRtosTcpStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGFREERTOSTCPSTREAMPRIVATE_H */
