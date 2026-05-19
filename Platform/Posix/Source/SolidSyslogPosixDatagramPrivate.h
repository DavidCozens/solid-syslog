#ifndef SOLIDSYSLOGPOSIXDATAGRAMPRIVATE_H
#define SOLIDSYSLOGPOSIXDATAGRAMPRIVATE_H

#include <stdbool.h>

#include "SolidSyslogDatagramDefinition.h"

struct SolidSyslogPosixDatagram
{
    struct SolidSyslogDatagram Base;
    int Fd;
    bool Connected;
};

void PosixDatagram_Initialise(struct SolidSyslogDatagram* base);
void PosixDatagram_Cleanup(struct SolidSyslogDatagram* base);

#endif /* SOLIDSYSLOGPOSIXDATAGRAMPRIVATE_H */
