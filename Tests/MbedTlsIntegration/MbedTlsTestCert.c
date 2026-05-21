#include "MbedTlsTestCert.h"

#include <mbedtls/asn1.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/oid.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>
#include <stddef.h>
#include <string.h>

#include "SafeString.h"

enum
{
    RSA_KEY_BITS = 2048,
    RSA_EXPONENT = 65537,
    DER_BUFFER_BYTES = 4096
};

/* mbedtls_x509write_crt_der writes from the END of the buffer backwards,
 * so the DER blob occupies [Bytes + StartOffset, Bytes + StartOffset + Length).
 * Capacity is the underlying storage size. Bundling these four fields keeps
 * WriteCertToDer's signature tight and dodges adjacent-size_t-parameter
 * swappability hazards. */
struct DerBuffer
{
    unsigned char* Bytes;
    size_t Capacity;
    size_t StartOffset;
    size_t Length;
};

static void GenerateKey(mbedtls_pk_context* key, mbedtls_ctr_drbg_context* rng);
static void WriteCertToDer(
    const struct MbedTlsTestCertConfig* config,
    mbedtls_pk_context* subjectKey,
    mbedtls_ctr_drbg_context* rng,
    struct DerBuffer* out
);

void MbedTlsTestCert_Create(
    const struct MbedTlsTestCertConfig* config,
    struct MbedTlsTestCert* out,
    mbedtls_ctr_drbg_context* rng
)
{
    mbedtls_pk_init(&out->Key);
    mbedtls_x509_crt_init(&out->Cert);
    SafeString_Copy(out->SubjectName, sizeof(out->SubjectName), config->SubjectName);

    GenerateKey(&out->Key, rng);

    unsigned char derBytes[DER_BUFFER_BYTES];
    struct DerBuffer der = {derBytes, sizeof(derBytes), 0U, 0U};
    WriteCertToDer(config, &out->Key, rng, &der);

    mbedtls_x509_crt_parse_der(&out->Cert, &der.Bytes[der.StartOffset], der.Length);
}

void MbedTlsTestCert_Destroy(struct MbedTlsTestCert* cert)
{
    mbedtls_x509_crt_free(&cert->Cert);
    mbedtls_pk_free(&cert->Key);
}

static void GenerateKey(mbedtls_pk_context* key, mbedtls_ctr_drbg_context* rng)
{
    mbedtls_pk_setup(key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    mbedtls_rsa_gen_key(mbedtls_pk_rsa(*key), mbedtls_ctr_drbg_random, rng, RSA_KEY_BITS, RSA_EXPONENT);
}

static void WriteCertToDer(
    const struct MbedTlsTestCertConfig* config,
    mbedtls_pk_context* subjectKey,
    mbedtls_ctr_drbg_context* rng,
    struct DerBuffer* out
)
{
    mbedtls_x509write_cert crt;
    mbedtls_x509write_crt_init(&crt);
    mbedtls_x509write_crt_set_md_alg(&crt, MBEDTLS_MD_SHA256);
    mbedtls_x509write_crt_set_version(&crt, MBEDTLS_X509_CRT_VERSION_3);
    mbedtls_x509write_crt_set_subject_key(&crt, subjectKey);
    mbedtls_x509write_crt_set_subject_name(&crt, config->SubjectName);

    /* Self-signed when Issuer == NULL — issuer name + key match the subject. */
    const char* issuerName = (config->Issuer != NULL) ? config->Issuer->SubjectName : config->SubjectName;
    mbedtls_pk_context* issuerKey = (config->Issuer != NULL) ? (mbedtls_pk_context*) &config->Issuer->Key : subjectKey;
    mbedtls_x509write_crt_set_issuer_name(&crt, issuerName);
    mbedtls_x509write_crt_set_issuer_key(&crt, issuerKey);

    const unsigned char serial[] = {0x01};
    mbedtls_x509write_crt_set_serial_raw(&crt, (unsigned char*) serial, sizeof(serial));

    /* Long-validity test certs: 2024-01-01 to 2099-01-01. */
    mbedtls_x509write_crt_set_validity(&crt, "20240101000000", "20990101000000");

    mbedtls_x509write_crt_set_basic_constraints(&crt, config->IsCa, -1);

    mbedtls_x509_san_list san;
    if (config->SubjectAltDns != NULL)
    {
        memset(&san, 0, sizeof(san));
        san.node.type = MBEDTLS_X509_SAN_DNS_NAME;
        san.node.san.unstructured_name.tag = MBEDTLS_ASN1_IA5_STRING;
        san.node.san.unstructured_name.p = (unsigned char*) config->SubjectAltDns;
        san.node.san.unstructured_name.len = strlen(config->SubjectAltDns);
        san.next = NULL;
        mbedtls_x509write_crt_set_subject_alternative_name(&crt, &san);
    }

    /* mbedtls_x509write_crt_der writes from the END of the buffer backwards. */
    int written = mbedtls_x509write_crt_der(&crt, out->Bytes, out->Capacity, mbedtls_ctr_drbg_random, rng);
    if (written > 0)
    {
        out->StartOffset = out->Capacity - (size_t) written;
        out->Length = (size_t) written;
    }

    mbedtls_x509write_crt_free(&crt);
}
