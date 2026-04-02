#include "CppUTest/TestHarness.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SenderSpy.h"
#include <cstdlib>
#include <string>

// clang-format off
static const char * const TEST_PRIVAL    = "<134>";
static const char * const TEST_TIMESTAMP = "2009-03-23T00:00:00.000Z";
static const char * const TEST_HOSTNAME  = "TestHost";
static const char * const TEST_APP_NAME  = "TestApp";
static const char * const TEST_PROCID    = "42";
static const char * const TEST_MSGID     = "54";
static const char * const TEST_SDATA     = "-";
static const char * const TEST_MSG       = "hello world";

static const int SYSLOG_FIELD_HEADER    = 0;
static const int SYSLOG_FIELD_TIMESTAMP = 1;
static const int SYSLOG_FIELD_HOSTNAME  = 2;
static const int SYSLOG_FIELD_APP_NAME  = 3;
static const int SYSLOG_FIELD_PROCID    = 4;
static const int SYSLOG_FIELD_MSGID     = 5;
static const int SYSLOG_FIELD_SDATA     = 6;
// clang-format on

static std::string SyslogField(const char* buffer, int n)
{
    std::string            s(buffer);
    std::string::size_type pos = 0;
    for (int i = 0; i < n; i++)
    {
        pos = s.find(' ', pos);
        if (pos == std::string::npos)
        {
            return {};
        }
        pos++;
    }
    std::string::size_type end = s.find(' ', pos);
    return s.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

static std::string SyslogMsg(const char* buffer)
{
    std::string            s(buffer);
    std::string::size_type pos = 0;
    for (int i = 0; i < SYSLOG_FIELD_SDATA + 1; i++)
    {
        pos = s.find(' ', pos);
        if (pos == std::string::npos)
        {
            return {};
        }
        pos++;
    }
    return s.substr(pos);
}

// clang-format off
TEST_GROUP(SolidSyslog)
{
    SolidSyslogConfig config;
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope and constness managed by CppUTest macro
    // cppcheck-suppress constVariablePointer -- SolidSyslog_Log requires non-const; false positive from macro expansion
    SolidSyslog *logger;

    void setup() override
    {
        SenderSpy_Reset();
        config = {SenderSpy_GetSender(), malloc, free, nullptr};
        logger = SolidSyslog_Create(&config);
    }

    void teardown() override
    {
        SolidSyslog_Destroy(logger);
    }

    void Log() const
    {
        struct SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO};
        SolidSyslog_Log(logger, &message);
    }

    void Log(enum SolidSyslog_Facility facility) const
    {
        struct SolidSyslogMessage message = {facility, SOLIDSYSLOG_SEVERITY_INFO};
        SolidSyslog_Log(logger, &message);
    }

    void Log(enum SolidSyslog_Severity severity) const
    {
        struct SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, severity};
        SolidSyslog_Log(logger, &message);
    }

    void Log(enum SolidSyslog_Facility facility, enum SolidSyslog_Severity severity) const
    {
        struct SolidSyslogMessage message = {facility, severity};
        SolidSyslog_Log(logger, &message);
    }

    static const char *LastMessage()
    {
        return SenderSpy_LastBufferAsString();
    }
};

// clang-format on

TEST(SolidSyslog, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslog, CreateReturnsNonNullHandle)
{
    CHECK(logger != nullptr);
}

TEST(SolidSyslog, TwoCreatesReturnDifferentHandles)
{
    SolidSyslog* second = SolidSyslog_Create(&config);
    CHECK(logger != second);
    SolidSyslog_Destroy(second);
}

TEST(SolidSyslog, EachLoggerSendsThroughItsOwnSender)
{
    SolidSyslogConfig secondConfig = {SenderSpy_GetSender(), malloc, free, nullptr};
    SolidSyslog*      second       = SolidSyslog_Create(&secondConfig);

    struct SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO};
    SolidSyslog_Log(logger, &message);
    SolidSyslog_Log(second, &message);
    LONGS_EQUAL(2, SenderSpy_CallCount());

    SolidSyslog_Destroy(second);
}

TEST(SolidSyslog, NoMessagesAreSentWhenLogIsNotCalled)
{
    LONGS_EQUAL(0, SenderSpy_CallCount());
}

TEST(SolidSyslog, SingleLogCallResultsInOneSend)
{
    Log();
    LONGS_EQUAL(1, SenderSpy_CallCount());
}

TEST(SolidSyslog, PriValIs134)
{
    Log();
    STRNCMP_EQUAL(TEST_PRIVAL, SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), strlen(TEST_PRIVAL));
}

TEST(SolidSyslog, FacilityAppearsInPrival)
{
    Log(SOLIDSYSLOG_FACILITY_NEWS);
    STRNCMP_EQUAL("<62>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 4);
}

TEST(SolidSyslog, SeverityAppearsInPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_CRIT);
    STRNCMP_EQUAL("<130>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 5);
}

TEST(SolidSyslog, LowestFacilityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_FACILITY_KERN);
    STRNCMP_EQUAL("<6>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 3);
}

TEST(SolidSyslog, HighestFacilityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_FACILITY_LOCAL7);
    STRNCMP_EQUAL("<190>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 5);
}

TEST(SolidSyslog, LowestSeverityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_EMERG);
    STRNCMP_EQUAL("<128>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 5);
}

TEST(SolidSyslog, HighestSeverityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_DEBUG);
    STRNCMP_EQUAL("<135>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 5);
}

TEST(SolidSyslog, OutOfRangeFacilityProducesErrorPrival)
{
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) -- intentionally testing out-of-range input
    struct SolidSyslogMessage message = {(enum SolidSyslog_Facility) 24, SOLIDSYSLOG_SEVERITY_INFO};
    SolidSyslog_Log(logger, &message);
    STRNCMP_EQUAL("<43>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 4);
}

TEST(SolidSyslog, OutOfRangeSeverityProducesErrorPrival)
{
    enum SolidSyslog_Severity invalid = SOLIDSYSLOG_SEVERITY_DEBUG;
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) -- intentionally testing out-of-range input
    invalid                           = static_cast<enum SolidSyslog_Severity>(static_cast<int>(invalid) + 1);
    struct SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, invalid};
    SolidSyslog_Log(logger, &message);
    STRNCMP_EQUAL("<43>", SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), 4);
}

TEST(SolidSyslog, VersionIs1)
{
    Log();
    std::string header         = SyslogField(LastMessage(), SYSLOG_FIELD_HEADER);
    auto        closingBracket = header.find('>');
    BYTES_EQUAL('1', header.at(closingBracket + 1));
}

IGNORE_TEST(SolidSyslog, TimestampIs20090323)
{
    Log();
    STRCMP_EQUAL(TEST_TIMESTAMP, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str());
}

TEST(SolidSyslog, HostnameIsTestHost)
{
    Log();
    STRCMP_EQUAL(TEST_HOSTNAME, SyslogField(LastMessage(), SYSLOG_FIELD_HOSTNAME).c_str());
}

TEST(SolidSyslog, AppNameIsTestApp)
{
    Log();
    STRCMP_EQUAL(TEST_APP_NAME, SyslogField(LastMessage(), SYSLOG_FIELD_APP_NAME).c_str());
}

TEST(SolidSyslog, ProcIdIs42)
{
    Log();
    STRCMP_EQUAL(TEST_PROCID, SyslogField(LastMessage(), SYSLOG_FIELD_PROCID).c_str());
}

TEST(SolidSyslog, MsgIdIs54)
{
    Log();
    STRCMP_EQUAL(TEST_MSGID, SyslogField(LastMessage(), SYSLOG_FIELD_MSGID).c_str());
}

TEST(SolidSyslog, StructuredDataIsNilValue)
{
    Log();
    STRCMP_EQUAL(TEST_SDATA, SyslogField(LastMessage(), SYSLOG_FIELD_SDATA).c_str());
}

TEST(SolidSyslog, MsgIsHelloWorld)
{
    Log();
    STRCMP_EQUAL(TEST_MSG, SyslogMsg(LastMessage()).c_str());
}

static void* AlwaysFailAlloc(size_t size)
{
    (void) size;
    return nullptr;
}

TEST(SolidSyslog, CreateReturnsNullWhenAllocFails)
{
    SolidSyslogConfig failConfig = {SenderSpy_GetSender(), AlwaysFailAlloc, free, nullptr};
    SolidSyslog*      result     = SolidSyslog_Create(&failConfig);
    POINTERS_EQUAL(nullptr, result);
}

static struct SolidSyslogTimestamp stubTimestamp;

static struct SolidSyslogTimestamp StubClock(void)
{
    return stubTimestamp;
}

// clang-format off
TEST_GROUP_BASE(SolidSyslogTimestamp, TEST_GROUP_CppUTestGroupSolidSyslog)
{
    void setup() override
    {
        TEST_GROUP_CppUTestGroupSolidSyslog::setup();
        stubTimestamp = {2026, 4, 2, 14, 30, 0, 0, 0};
        config.clock = StubClock;
        SolidSyslog_Destroy(logger);
        logger = SolidSyslog_Create(&config);
    }
};

// clang-format on

TEST(SolidSyslogTimestamp, NullClockProducesNilvalue)
{
    config.clock = nullptr;
    SolidSyslog_Destroy(logger);
    logger = SolidSyslog_Create(&config);
    Log();
    STRCMP_EQUAL("-", SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str());
}

TEST(SolidSyslogTimestamp, YearFormatsAsFourDigitZeroPadded)
{
    stubTimestamp.year = 2026;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("2026", timestamp.c_str(), 4);
}

TEST(SolidSyslogTimestamp, MonthFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.month = 4;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("04", timestamp.c_str() + 5, 2);
}

TEST(SolidSyslogTimestamp, DayFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.day = 2;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("02", timestamp.c_str() + 8, 2);
}

TEST(SolidSyslogTimestamp, HourFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.hour = 14;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("14", timestamp.c_str() + 11, 2);
}

TEST(SolidSyslogTimestamp, MinuteFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.minute = 30;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("30", timestamp.c_str() + 14, 2);
}

TEST(SolidSyslogTimestamp, SecondFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.second = 7;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("07", timestamp.c_str() + 17, 2);
}

TEST(SolidSyslogTimestamp, MicrosecondFormatsAsSixDigitZeroPadded)
{
    stubTimestamp.microsecond = 42;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL(".000042", timestamp.c_str() + 19, 7);
}

TEST(SolidSyslogTimestamp, DateFieldsSeparatedByHyphen)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('-', timestamp.at(4));
    BYTES_EQUAL('-', timestamp.at(7));
}

TEST(SolidSyslogTimestamp, DateAndTimeSeparatedByT)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('T', timestamp.at(10));
}

TEST(SolidSyslogTimestamp, TimeFieldsSeparatedByColon)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL(':', timestamp.at(13));
    BYTES_EQUAL(':', timestamp.at(16));
}

TEST(SolidSyslogTimestamp, FractionalSecondsPrecededByDot)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('.', timestamp.at(19));
}

TEST(SolidSyslogTimestamp, TimestampAppearsInCorrectMessageFieldPosition)
{
    stubTimestamp = {2026, 4, 2, 14, 30, 0, 0, 0};
    Log();
    STRCMP_EQUAL("2026-04-02T14:30:00.000000Z", SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str());
}

TEST(SolidSyslogTimestamp, ZeroOffsetFormatsAsZ)
{
    stubTimestamp.utcOffsetMinutes = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('Z', timestamp.at(timestamp.size() - 1));
}

TEST(SolidSyslogTimestamp, PositiveOffsetFormatsAsPlusHHMM)
{
    stubTimestamp.utcOffsetMinutes = 330;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    std::string offset    = timestamp.substr(26);
    STRCMP_EQUAL("+05:30", offset.c_str());
}

TEST(SolidSyslogTimestamp, NegativeOffsetFormatsAsMinusHHMM)
{
    stubTimestamp.utcOffsetMinutes = -300;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    std::string offset    = timestamp.substr(26);
    STRCMP_EQUAL("-05:00", offset.c_str());
}

TEST(SolidSyslogTimestamp, YearZeroFormatsAs0000)
{
    stubTimestamp.year = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("0000", timestamp.c_str(), 4);
}

TEST(SolidSyslogTimestamp, Year9999FormatsAs9999)
{
    stubTimestamp.year = 9999;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("9999", timestamp.c_str(), 4);
}

TEST(SolidSyslogTimestamp, Month1FormatsAs01)
{
    stubTimestamp.month = 1;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("01", timestamp.c_str() + 5, 2);
}

TEST(SolidSyslogTimestamp, Month12FormatsAs12)
{
    stubTimestamp.month = 12;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("12", timestamp.c_str() + 5, 2);
}

TEST(SolidSyslogTimestamp, Day1FormatsAs01)
{
    stubTimestamp.day = 1;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("01", timestamp.c_str() + 8, 2);
}

TEST(SolidSyslogTimestamp, Day31FormatsAs31)
{
    stubTimestamp.day = 31;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("31", timestamp.c_str() + 8, 2);
}

TEST(SolidSyslogTimestamp, Hour0FormatsAs00)
{
    stubTimestamp.hour = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("00", timestamp.c_str() + 11, 2);
}

TEST(SolidSyslogTimestamp, Hour23FormatsAs23)
{
    stubTimestamp.hour = 23;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("23", timestamp.c_str() + 11, 2);
}

TEST(SolidSyslogTimestamp, Minute0FormatsAs00)
{
    stubTimestamp.minute = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("00", timestamp.c_str() + 14, 2);
}

TEST(SolidSyslogTimestamp, Minute59FormatsAs59)
{
    stubTimestamp.minute = 59;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("59", timestamp.c_str() + 14, 2);
}

TEST(SolidSyslogTimestamp, Second0FormatsAs00)
{
    stubTimestamp.second = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("00", timestamp.c_str() + 17, 2);
}

TEST(SolidSyslogTimestamp, Second59FormatsAs59)
{
    stubTimestamp.second = 59;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL("59", timestamp.c_str() + 17, 2);
}

TEST(SolidSyslogTimestamp, Microsecond0FormatsAs000000)
{
    stubTimestamp.microsecond = 0;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL(".000000", timestamp.c_str() + 19, 7);
}

TEST(SolidSyslogTimestamp, Microsecond999999FormatsAs999999)
{
    stubTimestamp.microsecond = 999999;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    STRNCMP_EQUAL(".999999", timestamp.c_str() + 19, 7);
}

TEST(SolidSyslogTimestamp, UtcOffsetPlus840FormatsAsPlus1400)
{
    stubTimestamp.utcOffsetMinutes = 840;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    std::string offset    = timestamp.substr(26);
    STRCMP_EQUAL("+14:00", offset.c_str());
}

TEST(SolidSyslogTimestamp, UtcOffsetMinus720FormatsAsMinus1200)
{
    stubTimestamp.utcOffsetMinutes = -720;
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    std::string offset    = timestamp.substr(26);
    STRCMP_EQUAL("-12:00", offset.c_str());
}

IGNORE_TEST(SolidSyslog, TimestampTestList)
{
    // S1.3 — Timestamp encoding (Story #18)
    //
    // Exceptions — out-of-range fields produce NILVALUE "-"
    //   Month 0 produces NILVALUE
    //   Month 13 produces NILVALUE
    //   Day 0 produces NILVALUE
    //   Day 32 produces NILVALUE
    //   Hour 24 produces NILVALUE
    //   Minute 60 produces NILVALUE
    //   Second 60 produces NILVALUE
    //   Microsecond 1000000 produces NILVALUE
    //   UTC offset +841 produces NILVALUE
    //   UTC offset -721 produces NILVALUE
}

IGNORE_TEST(SolidSyslog, HappyPathOnly)
{
    // Error handling not yet implemented — see Epic #31
    //   SolidSyslog_Create with a NULL config returns NULL
    //   SolidSyslog_Destroy with a NULL handle does not crash
    //   SolidSyslog_Log on NULL handle does nothing, does not crash
    //
    // Optional header fields not yet driven in — see Epic #8
    //   MSG is preceded by UTF-8 BOM
}
