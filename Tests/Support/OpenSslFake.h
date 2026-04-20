#ifndef OPENSSLFAKE_H
#define OPENSSLFAKE_H

#include "ExternC.h"

/* Forward-declared OpenSSL types — full definitions live in <openssl/ssl.h>. */
struct ssl_ctx_st;
struct ssl_st;

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

EXTERN_C_END

#endif /* OPENSSLFAKE_H */
