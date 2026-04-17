#ifndef SOLIDSYSLOGRESOLVER_H
#define SOLIDSYSLOGRESOLVER_H

#include "ExternC.h"
#include "SolidSyslogTransport.h"
#include <netinet/in.h>
#include <stdbool.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    bool SolidSyslogResolver_Resolve(struct SolidSyslogResolver * resolver, enum SolidSyslogTransport transport, struct sockaddr_in * result);

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVER_H */
