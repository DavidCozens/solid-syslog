#ifndef SOLIDSYSLOGRESOLVER_H
#define SOLIDSYSLOGRESOLVER_H

#include <stdbool.h>
#include <stdint.h>

#include "ExternC.h"
#include "SolidSyslogTransport.h"

struct SolidSyslogAddress;

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    /** Look @p host up and, on success, write the destination into @p result for
     *  a later Datagram or Stream to read. @p result is caller-owned storage (a
     *  platform SolidSyslogAddress from SolidSyslog{Posix,Winsock,PlusTcp,LwipRaw}
     *  Address_Create); the resolver only fills it in, and only when it returns
     *  true. A DNS-backed resolver may block for the lookup; a numeric-literal
     *  resolver does not. Some resolvers pin a fixed destination and ignore
     *  @p host and/or @p transport.
     *
     *  @retval false Not resolved; @p result is left untouched and the caller's
     *                unresolved-host path runs (the send is skipped and retried
     *                on a later SolidSyslog_Service). */
    bool SolidSyslogResolver_Resolve(
        struct SolidSyslogResolver * resolver,
        enum SolidSyslogTransport transport,
        const char* host,
        uint16_t port,
        struct SolidSyslogAddress* result
    );

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVER_H */
