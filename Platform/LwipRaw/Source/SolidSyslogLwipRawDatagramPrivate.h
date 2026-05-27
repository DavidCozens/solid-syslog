#ifndef SOLIDSYSLOGLWIPRAWDATAGRAMPRIVATE_H
#define SOLIDSYSLOGLWIPRAWDATAGRAMPRIVATE_H

#include "SolidSyslogDatagramDefinition.h"

struct SolidSyslogLwipRawDatagram
{
    struct SolidSyslogDatagram Base;
};

void LwipRawDatagram_Initialise(struct SolidSyslogDatagram* base);
void LwipRawDatagram_Cleanup(struct SolidSyslogDatagram* base);

#endif /* SOLIDSYSLOGLWIPRAWDATAGRAMPRIVATE_H */
