#ifndef SOLIDSYSLOGTLSSTREAM_H
#define SOLIDSYSLOGTLSSTREAM_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    struct SolidSyslogTlsStreamConfig
    {
        struct SolidSyslogStream* transport;    /* underlying byte stream — caller owns */
        const char*               caBundlePath; /* PEM file of trust anchors */
        const char*               serverName;   /* SNI + cert hostname check; NULL to skip */
        const char*               cipherList;   /* TLS 1.2 cipher list; NULL = OpenSSL default */
    };

    struct SolidSyslogStream* SolidSyslogTlsStream_Create(const struct SolidSyslogTlsStreamConfig* config);
    void                      SolidSyslogTlsStream_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGTLSSTREAM_H */
