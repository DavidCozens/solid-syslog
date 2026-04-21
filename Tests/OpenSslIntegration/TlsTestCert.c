#include "TlsTestCert.h"

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TLS_TEST_CERT_RSA_BITS     2048
#define TLS_TEST_CERT_DEFAULT_VALIDITY_SECONDS 3600

static void SetValidity(X509* cert, time_t notBefore, time_t notAfter);
static void SetSubject(X509* cert, const char* commonName);
static void AddSubjectAltNames(X509* cert, const char* const* dnsNames);

void TlsTestCert_Create(const struct TlsTestCertConfig* config, struct TlsTestCert* out)
{
    EVP_PKEY* key  = EVP_RSA_gen(TLS_TEST_CERT_RSA_BITS);
    X509*     cert = X509_new();

    X509_set_version(cert, 2); /* X.509 v3 */
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);

    SetValidity(cert, config->notBefore, config->notAfter);
    SetSubject(cert, config->commonName);
    AddSubjectAltNames(cert, config->subjectAltDnsNames);

    X509*     issuerCert = (config->issuer != NULL) ? config->issuer->cert : cert;
    EVP_PKEY* issuerKey  = (config->issuer != NULL) ? config->issuer->key : key;
    X509_set_issuer_name(cert, X509_get_subject_name(issuerCert));
    X509_set_pubkey(cert, key);
    X509_sign(cert, issuerKey, EVP_sha256());

    out->cert = cert;
    out->key  = key;
}

void TlsTestCert_Destroy(struct TlsTestCert* cert)
{
    if (cert->cert != NULL)
    {
        X509_free(cert->cert);
        cert->cert = NULL;
    }
    if (cert->key != NULL)
    {
        EVP_PKEY_free(cert->key);
        cert->key = NULL;
    }
}

void TlsTestCert_WritePemToFile(const struct TlsTestCert* cert, const char* path)
{
    FILE* file = fopen(path, "w");
    PEM_write_X509(file, cert->cert);
    fclose(file);
}

static void SetValidity(X509* cert, time_t notBefore, time_t notAfter)
{
    time_t now   = time(NULL);
    time_t start = (notBefore != 0) ? notBefore : now;
    time_t end   = (notAfter != 0) ? notAfter : now + TLS_TEST_CERT_DEFAULT_VALIDITY_SECONDS;
    X509_time_adj_ex(X509_getm_notBefore(cert), 0, 0, &start);
    X509_time_adj_ex(X509_getm_notAfter(cert), 0, 0, &end);
}

static void SetSubject(X509* cert, const char* commonName)
{
    X509_NAME* name = X509_get_subject_name(cert);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char*) commonName, -1, -1, 0);
}

static void AddSubjectAltNames(X509* cert, const char* const* dnsNames)
{
    if (dnsNames == NULL)
    {
        return;
    }
    STACK_OF(GENERAL_NAME)* sans = sk_GENERAL_NAME_new_null();
    for (int i = 0; dnsNames[i] != NULL; i++)
    {
        GENERAL_NAME*   gen = GENERAL_NAME_new();
        ASN1_IA5STRING* str = ASN1_IA5STRING_new();
        ASN1_STRING_set(str, dnsNames[i], -1);
        GENERAL_NAME_set0_value(gen, GEN_DNS, str);
        sk_GENERAL_NAME_push(sans, gen);
    }
    X509_add1_ext_i2d(cert, NID_subject_alt_name, sans, 0, 0);
    sk_GENERAL_NAME_pop_free(sans, GENERAL_NAME_free);
}
