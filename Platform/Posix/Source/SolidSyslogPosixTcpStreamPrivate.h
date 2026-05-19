#ifndef SOLIDSYSLOGPOSIXTCPSTREAMPRIVATE_H
#define SOLIDSYSLOGPOSIXTCPSTREAMPRIVATE_H

#include "SolidSyslogStreamDefinition.h"

struct SolidSyslogPosixTcpStream
{
    struct SolidSyslogStream Base;
    int Fd;
};

void PosixTcpStream_Initialise(struct SolidSyslogStream* base);
void PosixTcpStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGPOSIXTCPSTREAMPRIVATE_H */
