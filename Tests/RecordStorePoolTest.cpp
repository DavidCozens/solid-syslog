#include "CppUTest/TestHarness.h"

extern "C"
{
#include "RecordStorePrivate.h"
#include "SolidSyslogNullSecurityPolicy.h"
#include "SolidSyslogTunables.h"
}

// clang-format off
TEST_GROUP(RecordStorePool)
{
    struct SolidSyslogSecurityPolicy* policy = nullptr;
    struct RecordStore* pooled[SOLIDSYSLOG_BLOCK_STORE_POOL_SIZE] = {};
    struct RecordStore* overflow                                   = nullptr;

    void setup() override
    {
        policy = SolidSyslogNullSecurityPolicy_Get();
    }

    void teardown() override
    {
        for (auto*& slot : pooled)
        {
            RecordStore_Destroy(slot);
            slot = nullptr;
        }
        RecordStore_Destroy(overflow);
        overflow = nullptr;
    }

    void FillPool()
    {
        for (auto*& slot : pooled)
        {
            slot = RecordStore_Create(policy);
        }
    }
};

// clang-format on

TEST(RecordStorePool, CreateReturnsNonNullForFreshPool)
{
    struct RecordStore* slot = RecordStore_Create(policy);
    CHECK_TEXT(slot != nullptr, "first Create on empty pool returned NULL");
    RecordStore_Destroy(slot);
}

TEST(RecordStorePool, FillingPoolThenOverflowReturnsNull)
{
    FillPool();

    overflow = RecordStore_Create(policy);

    CHECK_TEXT(overflow == nullptr, "exhausted pool should return NULL, not a handle");
    for (auto* slot : pooled)
    {
        CHECK_TEXT(slot != nullptr, "pool slot was nullptr (FillPool failed?)");
    }
}

TEST(RecordStorePool, DestroyReleasesSlotForReuse)
{
    FillPool();

    RecordStore_Destroy(pooled[0]);
    pooled[0] = RecordStore_Create(policy);

    CHECK_TEXT(pooled[0] != nullptr, "reacquire after Destroy returned NULL");
}

TEST(RecordStorePool, DestroyOfNullIsSilentNoop)
{
    /* TU-internal classes return NULL on exhaustion (no shared null-object).
     * The only legitimate path to a NULL handle is a failed Create, and the
     * consumer's own error reporting covers that. _Destroy(NULL) must therefore
     * be a silent no-op. */
    RecordStore_Destroy(nullptr);
}
