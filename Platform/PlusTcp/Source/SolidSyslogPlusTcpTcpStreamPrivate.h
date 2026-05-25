#ifndef SOLIDSYSLOGPLUSTCPTCPSTREAMPRIVATE_H
#define SOLIDSYSLOGPLUSTCPTCPSTREAMPRIVATE_H

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

#include "SolidSyslogPlusTcpTcpStream.h"
#include "SolidSyslogStreamDefinition.h"

struct SolidSyslogPlusTcpTcpStream
{
    struct SolidSyslogStream Base;
    struct SolidSyslogPlusTcpTcpStreamConfig Config;
    Socket_t Socket;
};

void PlusTcpTcpStream_Initialise(
    struct SolidSyslogStream* base,
    const struct SolidSyslogPlusTcpTcpStreamConfig* config
);
void PlusTcpTcpStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGPLUSTCPTCPSTREAMPRIVATE_H */
