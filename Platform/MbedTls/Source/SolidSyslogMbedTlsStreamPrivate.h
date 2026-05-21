#ifndef SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H
#define SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H

#include <mbedtls/ssl.h>
#include <stdbool.h>

#include "SolidSyslogMbedTlsStream.h"
#include "SolidSyslogStreamDefinition.h"

struct SolidSyslogMbedTlsStream
{
    struct SolidSyslogStream Base;
    struct SolidSyslogMbedTlsStreamConfig Config;
    mbedtls_ssl_config SslConfig;
    mbedtls_ssl_context SslContext;
    /* Tracks whether Open has reached the point of mbedTLS state allocation.
     * Close guards its mbedtls_*_free calls on this so an unopened stream
     * can be destroyed without touching uninitialised structs. */
    bool IsOpen;
};

void MbedTlsStream_Initialise(struct SolidSyslogStream* base, const struct SolidSyslogMbedTlsStreamConfig* config);
void MbedTlsStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H */
