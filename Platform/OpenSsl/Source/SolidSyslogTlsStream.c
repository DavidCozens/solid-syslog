#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogTlsStream.h"

#include <openssl/ssl.h>

struct SolidSyslogTlsStream
{
    struct SolidSyslogStream          base;
    struct SolidSyslogTlsStreamConfig config;
};

static bool     Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr);
static SSL_CTX* CreateSslContext(const char* caBundlePath);
static BIO*     CreateTransportBio(void);

static struct SolidSyslogTlsStream instance;

struct SolidSyslogStream* SolidSyslogTlsStream_Create(const struct SolidSyslogTlsStreamConfig* config)
{
    instance.config    = *config;
    instance.base.Open = Open;
    return &instance.base;
}

void SolidSyslogTlsStream_Destroy(void)
{
}

static bool Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogTlsStream* stream = (struct SolidSyslogTlsStream*) self;
    SolidSyslogStream_Open(stream->config.transport, addr);
    SSL_CTX* ctx = CreateSslContext(stream->config.caBundlePath);
    SSL*     ssl = SSL_new(ctx);
    BIO*     bio = CreateTransportBio();
    SSL_set_bio(ssl, bio, bio);
    SSL_connect(ssl);
    return true;
}

static BIO* CreateTransportBio(void)
{
    BIO_METHOD* method = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "SolidSyslog transport BIO");
    return BIO_new(method);
}

static SSL_CTX* CreateSslContext(const char* caBundlePath)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_load_verify_locations(ctx, caBundlePath, NULL);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    return ctx;
}
