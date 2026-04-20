#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogTlsStream.h"

#include <openssl/ssl.h>

struct SolidSyslogTlsStream
{
    struct SolidSyslogStream          base;
    struct SolidSyslogTlsStreamConfig config;
    SSL_CTX*                          ctx;
    SSL*                              ssl;
};

static bool     Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr);
static bool     Send(struct SolidSyslogStream* self, const void* buffer, size_t size);
static void     Close(struct SolidSyslogStream* self);
static SSL_CTX* CreateSslContext(const char* caBundlePath);
static BIO*     CreateTransportBio(void);
static int      TransportBioRead(BIO* bio, char* buffer, int size);
static int      TransportBioWrite(BIO* bio, const char* buffer, int size);

static struct SolidSyslogTlsStream instance;

struct SolidSyslogStream* SolidSyslogTlsStream_Create(const struct SolidSyslogTlsStreamConfig* config)
{
    instance.config     = *config;
    instance.base.Open  = Open;
    instance.base.Send  = Send;
    instance.base.Close = Close;
    return &instance.base;
}

void SolidSyslogTlsStream_Destroy(void)
{
    if (instance.ctx != NULL)
    {
        SSL_CTX_free(instance.ctx);
        instance.ctx = NULL;
    }
}

static bool Open(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogTlsStream* stream = (struct SolidSyslogTlsStream*) self;
    if (!SolidSyslogStream_Open(stream->config.transport, addr))
    {
        return false;
    }
    stream->ctx = CreateSslContext(stream->config.caBundlePath);
    stream->ssl = SSL_new(stream->ctx);
    BIO* bio     = CreateTransportBio();
    BIO_set_data(bio, stream->config.transport);
    SSL_set_bio(stream->ssl, bio, bio);
    if (stream->config.serverName != NULL)
    {
        SSL_set_tlsext_host_name(stream->ssl, stream->config.serverName);
        SSL_set1_host(stream->ssl, stream->config.serverName);
    }
    return SSL_connect(stream->ssl) > 0;
}

static bool Send(struct SolidSyslogStream* self, const void* buffer, size_t size)
{
    struct SolidSyslogTlsStream* stream = (struct SolidSyslogTlsStream*) self;
    return SSL_write(stream->ssl, buffer, (int) size) > 0;
}

static void Close(struct SolidSyslogStream* self)
{
    struct SolidSyslogTlsStream* stream = (struct SolidSyslogTlsStream*) self;
    SSL_shutdown(stream->ssl);
    SSL_free(stream->ssl);
    SolidSyslogStream_Close(stream->config.transport);
}

static BIO* CreateTransportBio(void)
{
    BIO_METHOD* method = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "SolidSyslog transport BIO");
    BIO_meth_set_read(method, TransportBioRead);
    BIO_meth_set_write(method, TransportBioWrite);
    return BIO_new(method);
}

static int TransportBioRead(BIO* bio, char* buffer, int size)
{
    struct SolidSyslogStream* transport = (struct SolidSyslogStream*) BIO_get_data(bio);
    return (int) SolidSyslogStream_Read(transport, buffer, (size_t) size);
}

static int TransportBioWrite(BIO* bio, const char* buffer, int size)
{
    struct SolidSyslogStream* transport = (struct SolidSyslogStream*) BIO_get_data(bio);
    return SolidSyslogStream_Send(transport, buffer, (size_t) size) ? size : -1;
}

static SSL_CTX* CreateSslContext(const char* caBundlePath)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_load_verify_locations(ctx, caBundlePath, NULL);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    return ctx;
}
