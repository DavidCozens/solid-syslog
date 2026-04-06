#include "CppUTest/TestHarness.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogStructuredData.h"

#include <cstdlib>
#include <cstring>

static struct SolidSyslogTimeQuality stubTimeQuality;

static void StubGetTimeQuality(struct SolidSyslogTimeQuality* timeQuality)
{
    *timeQuality = stubTimeQuality;
}

// clang-format off
TEST_GROUP(SolidSyslogTimeQualitySd)
{
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    SolidSyslogStructuredData* sd;
    char buffer[256];

    void setup() override
    {
        stubTimeQuality = {true, true, SOLIDSYSLOG_SYNC_ACCURACY_OMIT};
        sd = SolidSyslogTimeQualitySd_Create(malloc, StubGetTimeQuality);
    }

    void teardown() override
    {
        SolidSyslogTimeQualitySd_Destroy(sd, free);
    }

    size_t Format()
    {
        return SolidSyslogStructuredData_Format(sd, buffer, sizeof(buffer));
    }
};

// clang-format on

TEST(SolidSyslogTimeQualitySd, CreateReturnsNonNull)
{
    CHECK(sd != nullptr);
}

TEST(SolidSyslogTimeQualitySd, FormatProducesTzKnownAndIsSynced)
{
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatWithFalseValues)
{
    stubTimeQuality.tzKnown  = false;
    stubTimeQuality.isSynced = false;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"0\" isSynced=\"0\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatIncludesSyncAccuracyWhenNonZero)
{
    stubTimeQuality.syncAccuracyMicroseconds = 50;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"50\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, SyncAccuracyOfOneIsSmallestNonOmitValue)
{
    stubTimeQuality.syncAccuracyMicroseconds = 1;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, SyncAccuracyAtMaxUint32)
{
    stubTimeQuality.syncAccuracyMicroseconds = UINT32_MAX;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"4294967295\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, OmitSyncAccuracyUsesDefinedConstant)
{
    stubTimeQuality.syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, CallbackIsInvokedOnEachFormat)
{
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);

    stubTimeQuality.isSynced = false;
    Format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"0\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatReturnsLengthOfFormattedString)
{
    size_t len = Format();
    LONGS_EQUAL(strlen(buffer), len);
}

TEST(SolidSyslogTimeQualitySd, FormatReturnsLengthWithSyncAccuracy)
{
    stubTimeQuality.syncAccuracyMicroseconds = 50;
    size_t len                               = Format();
    LONGS_EQUAL(strlen(buffer), len);
}

TEST(SolidSyslogTimeQualitySd, DestroyDoesNotCrash)
{
    // Covered by teardown — this test documents the intent
}
