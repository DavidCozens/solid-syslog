#include <cstring>

#include "CppUTest/TestHarness.h"

extern "C"
{
#include "ConfigLockFake.h"
#include "ErrorHandlerFake.h"
#include "SolidSyslogMbedTlsHmacSha256Policy.h"
#include "SolidSyslogMbedTlsHmacSha256PolicyErrors.h"
#include "SolidSyslogNullSecurityPolicy.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogSecurityPolicyDefinition.h"
#include "SolidSyslogTunables.h"
}

#include "TestUtils.h"

using namespace CososoTesting;

enum
{
    HMAC_SHA256_TAG_SIZE = 32,
    TEST_KEY_SIZE = 32,
    TEST_KEY_BYTE = 0x2B
};

/* A valid key accessor — slice 3 never invokes it (the seal/verify stubs are
 * no-ops), but Create requires a non-NULL GetKey. Writes a fixed key so the
 * signature is exercised honestly. */
static bool TestGetKey(void* context, uint8_t* keyOut, size_t capacity, size_t* keyLengthOut)
{
    (void) context;
    size_t written = (capacity < TEST_KEY_SIZE) ? capacity : (size_t) TEST_KEY_SIZE;
    memset(keyOut, TEST_KEY_BYTE, written);
    *keyLengthOut = written;
    return true;
}

// clang-format off
TEST_GROUP(SolidSyslogMbedTlsHmacSha256Policy)
{
    struct SolidSyslogMbedTlsHmacSha256PolicyConfig config = {};
    struct SolidSyslogSecurityPolicy* pooled[SOLIDSYSLOG_MBED_TLS_HMAC_SHA256_POLICY_POOL_SIZE] = {};
    struct SolidSyslogSecurityPolicy* overflow = nullptr;

    void setup() override
    {
        config.GetKey     = TestGetKey;
        config.KeyContext = nullptr;
    }

    void teardown() override
    {
        for (auto* handle : pooled)
        {
            if (handle != nullptr)
            {
                SolidSyslogMbedTlsHmacSha256Policy_Destroy(handle);
            }
        }
        if (overflow != nullptr)
        {
            SolidSyslogMbedTlsHmacSha256Policy_Destroy(overflow);
        }
        ConfigLockFake_Uninstall();
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);
        }
    }
};

// clang-format on

TEST(SolidSyslogMbedTlsHmacSha256Policy, CreateReturnsHandleDistinctFromFallback)
{
    struct SolidSyslogSecurityPolicy* handle = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);

    CHECK_TEXT(handle != nullptr, "Create returned nullptr");
    CHECK_TEXT(handle != SolidSyslogNullSecurityPolicy_Get(), "Create returned the Null fallback");

    SolidSyslogMbedTlsHmacSha256Policy_Destroy(handle);
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, IntegritySizeIsThirtyTwo)
{
    struct SolidSyslogSecurityPolicy* handle = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);

    LONGS_EQUAL(HMAC_SHA256_TAG_SIZE, handle->IntegritySize);

    SolidSyslogMbedTlsHmacSha256Policy_Destroy(handle);
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, FillingPoolThenOverflowReturnsNullFallback)
{
    FillPool();

    overflow = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);

    POINTERS_EQUAL(SolidSyslogNullSecurityPolicy_Get(), overflow);
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, ExhaustedCreateReportsError)
{
    ErrorHandlerFake_Install(nullptr);
    FillPool();

    overflow = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_ERROR, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&MbedTlsHmacSha256PolicyErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(MBEDTLSHMACSHA256POLICY_ERROR_POOL_EXHAUSTED, ErrorHandlerFake_LastCode());
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, NullConfigReturnsNullFallback)
{
    POINTERS_EQUAL(SolidSyslogNullSecurityPolicy_Get(), SolidSyslogMbedTlsHmacSha256Policy_Create(nullptr));
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, NullGetKeyReturnsNullFallback)
{
    config.GetKey = nullptr;

    POINTERS_EQUAL(SolidSyslogNullSecurityPolicy_Get(), SolidSyslogMbedTlsHmacSha256Policy_Create(&config));
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, BadConfigReportsError)
{
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogMbedTlsHmacSha256Policy_Create(nullptr);

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_ERROR, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&MbedTlsHmacSha256PolicyErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(MBEDTLSHMACSHA256POLICY_ERROR_BAD_CONFIG, ErrorHandlerFake_LastCode());
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, CreateAcquiresAndReleasesConfigLockOnFirstFreeSlot)
{
    ConfigLockFake_Install();

    pooled[0] = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, DestroyOfPooledHandleLocksOnce)
{
    pooled[0] = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);
    ConfigLockFake_Install();

    SolidSyslogMbedTlsHmacSha256Policy_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ConfigLockFake_Lock, ONCE);
    CALLED_FAKE(ConfigLockFake_Unlock, ONCE);
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, DestroyOfUnknownHandleReportsWarning)
{
    ErrorHandlerFake_Install(nullptr);
    struct SolidSyslogSecurityPolicy stranger = {};

    SolidSyslogMbedTlsHmacSha256Policy_Destroy(&stranger);

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    LONGS_EQUAL(SOLIDSYSLOG_SEVERITY_WARNING, ErrorHandlerFake_LastSeverity());
    POINTERS_EQUAL(&MbedTlsHmacSha256PolicyErrorSource, ErrorHandlerFake_LastSource());
    UNSIGNED_LONGS_EQUAL(MBEDTLSHMACSHA256POLICY_ERROR_UNKNOWN_DESTROY, ErrorHandlerFake_LastCode());
}

TEST(SolidSyslogMbedTlsHmacSha256Policy, DestroyOfStaleHandleReportsWarning)
{
    pooled[0] = SolidSyslogMbedTlsHmacSha256Policy_Create(&config);
    SolidSyslogMbedTlsHmacSha256Policy_Destroy(pooled[0]);
    ErrorHandlerFake_Install(nullptr);

    SolidSyslogMbedTlsHmacSha256Policy_Destroy(pooled[0]);
    pooled[0] = nullptr;

    CALLED_FAKE(ErrorHandlerFake_Handle, ONCE);
    UNSIGNED_LONGS_EQUAL(MBEDTLSHMACSHA256POLICY_ERROR_UNKNOWN_DESTROY, ErrorHandlerFake_LastCode());
}
