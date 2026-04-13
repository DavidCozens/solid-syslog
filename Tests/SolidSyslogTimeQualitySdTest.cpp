#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogStructuredData.h"

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
    SolidSyslogFormatter formatter;

    void setup() override
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
        stubTimeQuality = {true, true, SOLIDSYSLOG_SYNC_ACCURACY_OMIT};
        sd = SolidSyslogTimeQualitySd_Create(StubGetTimeQuality);
    }

    void teardown() override
    {
        SolidSyslogTimeQualitySd_Destroy();
    }

    void format()
    {
        SolidSyslogStructuredData_Format(sd, &formatter);
    }

    void resetFormatter()
    {
        memset(buffer, 0, sizeof(buffer));
        SolidSyslogFormatter_Create(&formatter, buffer, sizeof(buffer));
    }
};

// clang-format on

TEST(SolidSyslogTimeQualitySd, CreateReturnsNonNull)
{
    CHECK(sd != nullptr);
}

TEST(SolidSyslogTimeQualitySd, FormatProducesTzKnownAndIsSynced)
{
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatWithFalseValues)
{
    stubTimeQuality.tzKnown  = false;
    stubTimeQuality.isSynced = false;
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"0\" isSynced=\"0\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatIncludesSyncAccuracyWhenNonZero)
{
    stubTimeQuality.syncAccuracyMicroseconds = 50;
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"50\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, SyncAccuracyOfOneIsSmallestNonOmitValue)
{
    stubTimeQuality.syncAccuracyMicroseconds = 1;
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, SyncAccuracyAtMaxUint32)
{
    stubTimeQuality.syncAccuracyMicroseconds = UINT32_MAX;
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\" syncAccuracy=\"4294967295\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, OmitSyncAccuracyUsesDefinedConstant)
{
    stubTimeQuality.syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, CallbackIsInvokedOnEachFormat)
{
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"1\"]", buffer);

    stubTimeQuality.isSynced = false;
    resetFormatter();
    format();
    STRCMP_EQUAL("[timeQuality tzKnown=\"1\" isSynced=\"0\"]", buffer);
}

TEST(SolidSyslogTimeQualitySd, FormatAdvancesFormatterPosition)
{
    format();
    LONGS_EQUAL(strlen(buffer), formatter.position);
}

TEST(SolidSyslogTimeQualitySd, FormatAdvancesPositionWithSyncAccuracy)
{
    stubTimeQuality.syncAccuracyMicroseconds = 50;
    format();
    LONGS_EQUAL(strlen(buffer), formatter.position);
}

TEST(SolidSyslogTimeQualitySd, DestroyDoesNotCrash)
{
    // Covered by teardown — this test documents the intent
}
