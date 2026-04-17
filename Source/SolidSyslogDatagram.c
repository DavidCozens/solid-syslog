#include "SolidSyslogDatagramDefinition.h"

void SolidSyslogDatagram_Open(struct SolidSyslogDatagram* datagram)
{
    datagram->Open(datagram);
}

bool SolidSyslogDatagram_SendTo(struct SolidSyslogDatagram* datagram, const void* buffer, size_t size, const struct sockaddr_in* addr)
{
    return datagram->SendTo(datagram, buffer, size, addr);
}

void SolidSyslogDatagram_Close(struct SolidSyslogDatagram* datagram)
{
    datagram->Close(datagram);
}
