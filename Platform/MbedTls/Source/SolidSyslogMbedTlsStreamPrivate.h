#ifndef SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H
#define SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H

#include <mbedtls/ssl.h>

#include "SolidSyslogMbedTlsStream.h"
#include "SolidSyslogStreamDefinition.h"

struct SolidSyslogMbedTlsStream
{
    struct SolidSyslogStream Base;
    struct SolidSyslogMbedTlsStreamConfig Config;
    mbedtls_ssl_config SslConfig;
    mbedtls_ssl_context SslContext;
};

void MbedTlsStream_Initialise(struct SolidSyslogStream* base, const struct SolidSyslogMbedTlsStreamConfig* config);
void MbedTlsStream_Cleanup(struct SolidSyslogStream* base);

#endif /* SOLIDSYSLOGMBEDTLSSTREAMPRIVATE_H */
