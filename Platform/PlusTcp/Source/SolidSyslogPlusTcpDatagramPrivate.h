#ifndef SOLIDSYSLOGPLUSTCPDATAGRAMPRIVATE_H
#define SOLIDSYSLOGPLUSTCPDATAGRAMPRIVATE_H

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

#include "SolidSyslogDatagramDefinition.h"

struct SolidSyslogPlusTcpDatagram
{
    struct SolidSyslogDatagram Base;
    Socket_t Socket;
};

void PlusTcpDatagram_Initialise(struct SolidSyslogDatagram* base);
void PlusTcpDatagram_Cleanup(struct SolidSyslogDatagram* base);

#endif /* SOLIDSYSLOGPLUSTCPDATAGRAMPRIVATE_H */
