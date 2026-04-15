#ifndef SOLIDSYSLOGRESOLVER_H
#define SOLIDSYSLOGRESOLVER_H

#include "ExternC.h"
#include <netinet/in.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    void SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver,
                                      const char* host,
                                      int socktype,
                                      struct sockaddr_in* result);

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVER_H */
