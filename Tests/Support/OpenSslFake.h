#ifndef OPENSSLFAKE_H
#define OPENSSLFAKE_H

#include "ExternC.h"

/* Forward-declared OpenSSL types — full definitions live in <openssl/ssl.h>. */
struct ssl_ctx_st;
struct ssl_st;
struct bio_st;

EXTERN_C_BEGIN

    void               OpenSslFake_Reset(void);
    int                OpenSslFake_CtxNewCallCount(void);
    struct ssl_ctx_st* OpenSslFake_LastCtxReturned(void);
    const char*        OpenSslFake_LastCaBundlePath(void);
    int                OpenSslFake_LastVerifyMode(void);
    long               OpenSslFake_LastMinProtoVersion(void);
    int                OpenSslFake_SslNewCallCount(void);
    struct ssl_st*     OpenSslFake_LastSslReturned(void);
    struct ssl_ctx_st* OpenSslFake_LastSslNewCtxArg(void);
    int                OpenSslFake_BioNewCallCount(void);
    struct bio_st*     OpenSslFake_LastBioReturned(void);
    int                OpenSslFake_SetBioCallCount(void);
    struct ssl_st*     OpenSslFake_LastSetBioSslArg(void);
    struct bio_st*     OpenSslFake_LastSetBioReadBioArg(void);
    int                OpenSslFake_ConnectCallCount(void);
    struct ssl_st*     OpenSslFake_LastConnectSslArg(void);
    const char*        OpenSslFake_LastSniHostname(void);
    const char*        OpenSslFake_LastSet1Host(void);
    void*              OpenSslFake_LastSetDataArg(void);
    int (*OpenSslFake_LastBioReadCallback(void))(struct bio_st*, char*, int);
    int (*OpenSslFake_LastBioWriteCallback(void))(struct bio_st*, const char*, int);

EXTERN_C_END

#endif /* OPENSSLFAKE_H */
