#ifndef SOLIDSYSLOGADDRESSINTERNAL_H
#define SOLIDSYSLOGADDRESSINTERNAL_H

#include "SolidSyslogAddress.h"

#include <netinet/in.h>
#include <stdint.h>

static inline struct sockaddr_in* SolidSyslogAddress_AsSockaddrIn(struct SolidSyslogAddress* address)
{
    uint8_t* bytes = (uint8_t*) address;
    return (struct sockaddr_in*) bytes;
}

static inline const struct sockaddr_in* SolidSyslogAddress_AsConstSockaddrIn(const struct SolidSyslogAddress* address)
{
    const uint8_t* bytes = (const uint8_t*) address;
    return (const struct sockaddr_in*) bytes;
}

#endif /* SOLIDSYSLOGADDRESSINTERNAL_H */
