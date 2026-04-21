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
#include <unistd.h>

// clang-format off
TEST_GROUP(TlsStreamIntegration)
{
    struct TlsTestCert                cert          = {};
    struct TlsTestServer*             server        = nullptr;
    struct SolidSyslogStream*         transport     = nullptr;
    struct SolidSyslogTlsStreamConfig tlsConfig     = {};
    struct SolidSyslogStream*         tlsStream     = nullptr;
    SolidSyslogAddressStorage         addrStorage   = {};
    struct SolidSyslogAddress*        addr          = nullptr;
    char                              caPath[64]    = {};

    void setup() override
    {
        static const char* const sans[] = { "localhost", nullptr };
        struct TlsTestCertConfig certConfig = {};
        certConfig.commonName         = "localhost";
        certConfig.subjectAltDnsNames = sans;
        TlsTestCert_Create(&certConfig, &cert);

        std::strcpy(caPath, "/tmp/solidsyslog_tls_ca_XXXXXX");
        int fd = mkstemp(caPath);
        close(fd);
        TlsTestCert_WritePemToFile(&cert, caPath);

        struct TlsTestServerConfig serverConfig = {};
        serverConfig.serverCert = &cert;
        server                  = TlsTestServer_Create(&serverConfig);

        transport = BioPairStream_Create(TlsTestServer_ClientSideBio(server));
        BioPairStream_SetPump(transport, TlsTestServer_Pump, server);

        tlsConfig.transport    = transport;
        tlsConfig.caBundlePath = caPath;
        tlsConfig.serverName   = "localhost";
        tlsStream              = SolidSyslogTlsStream_Create(&tlsConfig);

        addr = SolidSyslogAddress_FromStorage(&addrStorage);
    }

    void teardown() override
    {
        SolidSyslogTlsStream_Destroy();
        BioPairStream_Destroy(transport);
        TlsTestServer_Destroy(server);
        TlsTestCert_Destroy(&cert);
        unlink(caPath);
    }
};
// clang-format on

TEST(TlsStreamIntegration, HandshakeSucceedsAgainstTrustedServerCert)
{
    CHECK_TRUE(SolidSyslogStream_Open(tlsStream, addr));
}
