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

// clang-format off
static const int TIMESTAMP_YEAR_OFFSET        = 0;
static const int TIMESTAMP_YEAR_LENGTH        = 4;
static const int TIMESTAMP_MONTH_OFFSET       = 5;
static const int TIMESTAMP_MONTH_LENGTH       = 2;
static const int TIMESTAMP_DAY_OFFSET         = 8;
static const int TIMESTAMP_DAY_LENGTH         = 2;
static const int TIMESTAMP_HOUR_OFFSET        = 11;
static const int TIMESTAMP_HOUR_LENGTH        = 2;
static const int TIMESTAMP_MINUTE_OFFSET      = 14;
static const int TIMESTAMP_MINUTE_LENGTH      = 2;
static const int TIMESTAMP_SECOND_OFFSET      = 17;
static const int TIMESTAMP_SECOND_LENGTH      = 2;
static const int TIMESTAMP_DATE_SEP_1          = 4;
static const int TIMESTAMP_DATE_SEP_2          = 7;
static const int TIMESTAMP_DATE_TIME_SEP       = 10;
static const int TIMESTAMP_TIME_SEP_1          = 13;
static const int TIMESTAMP_TIME_SEP_2          = 16;
static const int TIMESTAMP_MICROSECOND_OFFSET  = 19;
static const int TIMESTAMP_MICROSECOND_LENGTH  = 7;
static const int TIMESTAMP_OFFSET_OFFSET       = 26;
// clang-format on

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- macros preserve __FILE__/__LINE__ in test failure output
#define CHECK_PRIVAL(expected)                                                         \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_HEADER).c_str(), strlen(expected))

#define CHECK_TIMESTAMP_YEAR(expected)                                                 \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_YEAR_OFFSET, TIMESTAMP_YEAR_LENGTH)

#define CHECK_TIMESTAMP_MONTH(expected)                                                \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_MONTH_OFFSET, TIMESTAMP_MONTH_LENGTH)

#define CHECK_TIMESTAMP_DAY(expected)                                                  \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_DAY_OFFSET, TIMESTAMP_DAY_LENGTH)

#define CHECK_TIMESTAMP_HOUR(expected)                                                 \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_HOUR_OFFSET, TIMESTAMP_HOUR_LENGTH)

#define CHECK_TIMESTAMP_MINUTE(expected)                                               \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_MINUTE_OFFSET, TIMESTAMP_MINUTE_LENGTH)

#define CHECK_TIMESTAMP_SECOND(expected)                                               \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_SECOND_OFFSET, TIMESTAMP_SECOND_LENGTH)

#define CHECK_TIMESTAMP_MICROSECOND(expected)                                          \
    STRNCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str() + TIMESTAMP_MICROSECOND_OFFSET, TIMESTAMP_MICROSECOND_LENGTH)

#define CHECK_TIMESTAMP(expected)                                                      \
    STRCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str())

#define CHECK_TIMESTAMP_OFFSET(expected)                                               \
    STRCMP_EQUAL(expected, SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).substr(TIMESTAMP_OFFSET_OFFSET).c_str())

#define CHECK_TIMESTAMP_IS_NILVALUE()                                                  \
    STRCMP_EQUAL("-", SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP).c_str())
// NOLINTEND(cppcoreguidelines-macro-usage)

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
    CHECK_PRIVAL(TEST_PRIVAL);
}

TEST(SolidSyslog, FacilityAppearsInPrival)
{
    Log(SOLIDSYSLOG_FACILITY_NEWS);
    CHECK_PRIVAL("<62>");
}

TEST(SolidSyslog, SeverityAppearsInPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_CRIT);
    CHECK_PRIVAL("<130>");
}

TEST(SolidSyslog, LowestFacilityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_FACILITY_KERN);
    CHECK_PRIVAL("<6>");
}

TEST(SolidSyslog, HighestFacilityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_FACILITY_LOCAL7);
    CHECK_PRIVAL("<190>");
}

TEST(SolidSyslog, LowestSeverityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_EMERG);
    CHECK_PRIVAL("<128>");
}

TEST(SolidSyslog, HighestSeverityProducesCorrectPrival)
{
    Log(SOLIDSYSLOG_SEVERITY_DEBUG);
    CHECK_PRIVAL("<135>");
}

TEST(SolidSyslog, OutOfRangeFacilityProducesErrorPrival)
{
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) -- intentionally testing out-of-range input
    struct SolidSyslogMessage message = {(enum SolidSyslog_Facility) 24, SOLIDSYSLOG_SEVERITY_INFO};
    SolidSyslog_Log(logger, &message);
    CHECK_PRIVAL("<43>");
}

TEST(SolidSyslog, OutOfRangeSeverityProducesErrorPrival)
{
    enum SolidSyslog_Severity invalid = SOLIDSYSLOG_SEVERITY_DEBUG;
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) -- intentionally testing out-of-range input
    invalid                           = static_cast<enum SolidSyslog_Severity>(static_cast<int>(invalid) + 1);
    struct SolidSyslogMessage message = {SOLIDSYSLOG_FACILITY_LOCAL0, invalid};
    SolidSyslog_Log(logger, &message);
    CHECK_PRIVAL("<43>");
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

// clang-format off
static const uint16_t TEST_YEAR        = 2026;
static const uint8_t  TEST_MONTH       = 4;
static const uint8_t  TEST_DAY         = 2;
static const uint8_t  TEST_HOUR        = 14;
static const uint8_t  TEST_MINUTE      = 30;
static const uint8_t  TEST_SECOND      = 7;
static const uint32_t TEST_MICROSECOND = 42;
static const int16_t  TEST_UTC_OFFSET  = 0;
// clang-format on

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
        stubTimestamp = {TEST_YEAR, TEST_MONTH, TEST_DAY, TEST_HOUR, TEST_MINUTE, TEST_SECOND, TEST_MICROSECOND, TEST_UTC_OFFSET};
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
    CHECK_TIMESTAMP_IS_NILVALUE();
}

TEST(SolidSyslogTimestamp, YearFormatsAsFourDigitZeroPadded)
{
    stubTimestamp.year = TEST_YEAR;
    Log();
    CHECK_TIMESTAMP_YEAR("2026");
}

TEST(SolidSyslogTimestamp, MonthFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.month = TEST_MONTH;
    Log();
    CHECK_TIMESTAMP_MONTH("04");
}

TEST(SolidSyslogTimestamp, DayFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.day = TEST_DAY;
    Log();
    CHECK_TIMESTAMP_DAY("02");
}

TEST(SolidSyslogTimestamp, HourFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.hour = TEST_HOUR;
    Log();
    CHECK_TIMESTAMP_HOUR("14");
}

TEST(SolidSyslogTimestamp, MinuteFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.minute = TEST_MINUTE;
    Log();
    CHECK_TIMESTAMP_MINUTE("30");
}

TEST(SolidSyslogTimestamp, SecondFormatsAsTwoDigitZeroPadded)
{
    stubTimestamp.second = TEST_SECOND;
    Log();
    CHECK_TIMESTAMP_SECOND("07");
}

TEST(SolidSyslogTimestamp, MicrosecondFormatsAsSixDigitZeroPadded)
{
    stubTimestamp.microsecond = TEST_MICROSECOND;
    Log();
    CHECK_TIMESTAMP_MICROSECOND(".000042");
}

TEST(SolidSyslogTimestamp, DateFieldsSeparatedByHyphen)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('-', timestamp.at(TIMESTAMP_DATE_SEP_1));
    BYTES_EQUAL('-', timestamp.at(TIMESTAMP_DATE_SEP_2));
}

TEST(SolidSyslogTimestamp, DateAndTimeSeparatedByT)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('T', timestamp.at(TIMESTAMP_DATE_TIME_SEP));
}

TEST(SolidSyslogTimestamp, TimeFieldsSeparatedByColon)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL(':', timestamp.at(TIMESTAMP_TIME_SEP_1));
    BYTES_EQUAL(':', timestamp.at(TIMESTAMP_TIME_SEP_2));
}

TEST(SolidSyslogTimestamp, FractionalSecondsPrecededByDot)
{
    Log();
    std::string timestamp = SyslogField(LastMessage(), SYSLOG_FIELD_TIMESTAMP);
    BYTES_EQUAL('.', timestamp.at(TIMESTAMP_MICROSECOND_OFFSET));
}

TEST(SolidSyslogTimestamp, TimestampAppearsInCorrectMessageFieldPosition)
{
    Log();
    CHECK_TIMESTAMP("2026-04-02T14:30:07.000042Z");
}

TEST(SolidSyslogTimestamp, ZeroOffsetFormatsAsZ)
{
    stubTimestamp.utcOffsetMinutes = TEST_UTC_OFFSET;
    Log();
    CHECK_TIMESTAMP_OFFSET("Z");
}

TEST(SolidSyslogTimestamp, PositiveOffsetFormatsAsPlusHHMM)
{
    stubTimestamp.utcOffsetMinutes = 330;
    Log();
    CHECK_TIMESTAMP_OFFSET("+05:30");
}

TEST(SolidSyslogTimestamp, NegativeOffsetFormatsAsMinusHHMM)
{
    stubTimestamp.utcOffsetMinutes = -300;
    Log();
    CHECK_TIMESTAMP_OFFSET("-05:00");
}

TEST(SolidSyslogTimestamp, YearZeroFormatsAs0000)
{
    stubTimestamp.year = 0;
    Log();
    CHECK_TIMESTAMP_YEAR("0000");
}

TEST(SolidSyslogTimestamp, Year9999FormatsAs9999)
{
    stubTimestamp.year = 9999;
    Log();
    CHECK_TIMESTAMP_YEAR("9999");
}

TEST(SolidSyslogTimestamp, Month1FormatsAs01)
{
    stubTimestamp.month = 1;
    Log();
    CHECK_TIMESTAMP_MONTH("01");
}

TEST(SolidSyslogTimestamp, Month12FormatsAs12)
{
    stubTimestamp.month = 12;
    Log();
    CHECK_TIMESTAMP_MONTH("12");
}

TEST(SolidSyslogTimestamp, Day1FormatsAs01)
{
    stubTimestamp.day = 1;
    Log();
    CHECK_TIMESTAMP_DAY("01");
}

TEST(SolidSyslogTimestamp, Day31FormatsAs31)
{
    stubTimestamp.day = 31;
    Log();
    CHECK_TIMESTAMP_DAY("31");
}

TEST(SolidSyslogTimestamp, Hour0FormatsAs00)
{
    stubTimestamp.hour = 0;
    Log();
    CHECK_TIMESTAMP_HOUR("00");
}

TEST(SolidSyslogTimestamp, Hour23FormatsAs23)
{
    stubTimestamp.hour = 23;
    Log();
    CHECK_TIMESTAMP_HOUR("23");
}

TEST(SolidSyslogTimestamp, Minute0FormatsAs00)
{
    stubTimestamp.minute = 0;
    Log();
    CHECK_TIMESTAMP_MINUTE("00");
}

TEST(SolidSyslogTimestamp, Minute59FormatsAs59)
{
    stubTimestamp.minute = 59;
    Log();
    CHECK_TIMESTAMP_MINUTE("59");
}

TEST(SolidSyslogTimestamp, Second0FormatsAs00)
{
    stubTimestamp.second = 0;
    Log();
    CHECK_TIMESTAMP_SECOND("00");
}

TEST(SolidSyslogTimestamp, Second59FormatsAs59)
{
    stubTimestamp.second = 59;
    Log();
    CHECK_TIMESTAMP_SECOND("59");
}

TEST(SolidSyslogTimestamp, Microsecond0FormatsAs000000)
{
    stubTimestamp.microsecond = 0;
    Log();
    CHECK_TIMESTAMP_MICROSECOND(".000000");
}

TEST(SolidSyslogTimestamp, Microsecond999999FormatsAs999999)
{
    stubTimestamp.microsecond = 999999;
    Log();
    CHECK_TIMESTAMP_MICROSECOND(".999999");
}

TEST(SolidSyslogTimestamp, UtcOffsetPlus840FormatsAsPlus1400)
{
    stubTimestamp.utcOffsetMinutes = 840;
    Log();
    CHECK_TIMESTAMP_OFFSET("+14:00");
}

TEST(SolidSyslogTimestamp, UtcOffsetMinus720FormatsAsMinus1200)
{
    stubTimestamp.utcOffsetMinutes = -720;
    Log();
    CHECK_TIMESTAMP_OFFSET("-12:00");
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
