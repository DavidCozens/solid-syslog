#include "BioPairStream.h"
#include "CppUTest/TestHarness.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogTlsStream.h"
#include "TlsTestCert.h"
#include "TlsTestServer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>

// clang-format off
TEST_GROUP(TlsStreamIntegration)
{
    struct TlsTestCert                cert        = {};
    struct TlsTestServer*             server      = nullptr;
    struct SolidSyslogStream*         transport   = nullptr;
    struct SolidSyslogTlsStreamConfig tlsConfig   = {};
    struct SolidSyslogStream*         tlsStream   = nullptr;
    SolidSyslogAddressStorage         addrStorage = {};
    struct SolidSyslogAddress*        addr        = nullptr;
    char                              caPath[64]  = {};

    void setup() override
    {
        addr = SolidSyslogAddress_FromStorage(&addrStorage);
    }

    void teardown() override
    {
        if (tlsStream != nullptr)    { SolidSyslogTlsStream_Destroy(); }
        if (transport != nullptr)    { BioPairStream_Destroy(transport); }
        if (server != nullptr)       { TlsTestServer_Destroy(server); }
        if (cert.cert != nullptr)    { TlsTestCert_Destroy(&cert); }
        if (caPath[0] != '\0')       { unlink(caPath); }
    }

    void buildScenario(const struct TlsTestCertConfig& certConfig,
                       const char*                     clientServerName = "localhost")
    {
        TlsTestCert_Create(&certConfig, &cert);
        std::strcpy(caPath, "/tmp/solidsyslog_tls_ca_XXXXXX");
        int fd = mkstemp(caPath);
        CHECK_TRUE(fd >= 0);
        if (fd < 0)
        {
            return;
        }
        close(fd);
        TlsTestCert_WritePemToFile(&cert, caPath);

        struct TlsTestServerConfig serverConfig = {};
        serverConfig.serverCert = &cert;
        server                  = TlsTestServer_Create(&serverConfig);

        transport = BioPairStream_Create(TlsTestServer_ClientSideBio(server));
        BioPairStream_SetPump(transport, TlsTestServer_Pump, server);

        tlsConfig.transport    = transport;
        tlsConfig.caBundlePath = caPath;
        tlsConfig.serverName   = clientServerName;
        tlsStream              = SolidSyslogTlsStream_Create(&tlsConfig);
    }
};

// clang-format on

static const char* const LOCALHOST_SANS[] = {"localhost", nullptr};

TEST(TlsStreamIntegration, HandshakeSucceedsAgainstTrustedServerCert)
{
    struct TlsTestCertConfig certConfig = {};
    certConfig.commonName               = "localhost";
    certConfig.subjectAltDnsNames       = LOCALHOST_SANS;
    buildScenario(certConfig);

    CHECK_TRUE(SolidSyslogStream_Open(tlsStream, addr));
}

TEST(TlsStreamIntegration, HandshakeRejectedWhenServerCertIsExpired)
{
    struct TlsTestCertConfig certConfig = {};
    certConfig.commonName               = "localhost";
    certConfig.subjectAltDnsNames       = LOCALHOST_SANS;
    certConfig.notBefore                = std::time(nullptr) - 7200;
    certConfig.notAfter                 = std::time(nullptr) - 3600;
    buildScenario(certConfig);

    CHECK_FALSE(SolidSyslogStream_Open(tlsStream, addr));
}

TEST(TlsStreamIntegration, HandshakeRejectedWhenServerCertHostnameDoesNotMatch)
{
    static const char* const otherSans[] = {"someone-else.example", nullptr};
    struct TlsTestCertConfig certConfig  = {};
    certConfig.commonName                = "someone-else.example";
    certConfig.subjectAltDnsNames        = otherSans;
    buildScenario(certConfig); /* client.serverName defaults to "localhost" */

    CHECK_FALSE(SolidSyslogStream_Open(tlsStream, addr));
}

TEST(TlsStreamIntegration, HandshakeRejectedWhenClientDoesNotTrustServerCert)
{
    struct TlsTestCertConfig certConfig = {};
    certConfig.commonName               = "localhost";
    certConfig.subjectAltDnsNames       = LOCALHOST_SANS;
    buildScenario(certConfig);

    /* Overwrite the client's trust file with an unrelated self-signed cert
     * so the server's cert is no longer anchored in the trust store. The CA
     * file is loaded on Open, so this replacement takes effect for the next
     * handshake attempt. */
    struct TlsTestCertConfig untrustedConfig = {};
    untrustedConfig.commonName               = "some-other-entity.example";
    struct TlsTestCert untrusted             = {};
    TlsTestCert_Create(&untrustedConfig, &untrusted);
    TlsTestCert_WritePemToFile(&untrusted, caPath);

    CHECK_FALSE(SolidSyslogStream_Open(tlsStream, addr));

    TlsTestCert_Destroy(&untrusted);
}

TEST(TlsStreamIntegration, HandshakeRejectedWhenCipherListIsUnsupported)
{
    struct TlsTestCertConfig certConfig = {};
    certConfig.commonName               = "localhost";
    certConfig.subjectAltDnsNames       = LOCALHOST_SANS;
    tlsConfig.cipherList                = "NOT-A-REAL-CIPHER";
    buildScenario(certConfig);

    CHECK_FALSE(SolidSyslogStream_Open(tlsStream, addr));
}
