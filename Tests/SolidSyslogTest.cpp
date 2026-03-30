#include "CppUTest/TestHarness.h"
#include "SolidSyslog.h"
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
    SolidSyslog_Config config;
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope and constness managed by CppUTest macro
    // cppcheck-suppress constVariablePointer -- SolidSyslog_Log requires non-const; false positive from macro expansion
    SolidSyslog *logger;

    void setup() override
    {
        SenderSpy_Reset();
        config = {SenderSpy_GetSender(), malloc, free};
        logger = SolidSyslog_Create(&config);
    }

    void teardown() override
    {
        SolidSyslog_Destroy(logger);
    }

    void Log() const
    {
        SolidSyslog_Log(logger);
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
    SolidSyslog_Config secondConfig = {SenderSpy_GetSender(), malloc, free};
    SolidSyslog*       second       = SolidSyslog_Create(&secondConfig);

    SolidSyslog_Log(logger);
    SolidSyslog_Log(second);
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

TEST(SolidSyslog, VersionIs1)
{
    Log();
    std::string header         = SyslogField(LastMessage(), SYSLOG_FIELD_HEADER);
    auto        closingBracket = header.find('>');
    BYTES_EQUAL('1', header.at(closingBracket + 1));
}

TEST(SolidSyslog, TimestampIs20090323)
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
    SolidSyslog_Config failConfig = {SenderSpy_GetSender(), AlwaysFailAlloc, free};
    SolidSyslog*       result     = SolidSyslog_Create(&failConfig);
    POINTERS_EQUAL(nullptr, result);
}

// clang-format off
// Test list — S1.1: Walking Skeleton
//
// Test defaults (hard-coded for walking skeleton, driven in by later stories):
//   PRIVAL    : 134  (facility=LOCAL0=16, severity=INFO=6, 16*8+6=134)
//   VERSION   : 1    (fixed by RFC 5424)
//   TIMESTAMP : 2009-03-23T00:00:00.000Z  (RFC 5424 publication date)
//   HOSTNAME  : TestHost
//   APP-NAME  : TestApp
//   PROCID    : 42
//   MSGID     : 54
//   SDATA     : -    (NILVALUE)
//   MSG       : hello world
//
//   Full expected message:
//   <134>1 2009-03-23T00:00:00.000Z TestHost TestApp 42 54 - hello world
//
// Z — Zero
//   [x] No messages are sent when log is not called
//
// O — One
//   [x] A single SolidSyslog_Log(logger) call results in exactly one Send on the spy
//   [x] The PRIVAL value is 134 (tested as '<134>')
//   [x] The VERSION field is '1'
//   [x] The TIMESTAMP field is '2009-03-23T00:00:00.000Z'
//   [x] The HOSTNAME field is 'TestHost'
//   [x] The APP-NAME field is 'TestApp'
//   [x] The PROCID field is '42'
//   [x] The MSGID field is '54'
//   [x] The STRUCTURED-DATA field is '-'
//   [x] The MSG field is 'hello world'
//   [ ] (S1.x) MSG is preceded by UTF-8 BOM
//
// B — Boundaries
//   [ ] SolidSyslog_Log called twice results in Send called twice
//
// I — Interfaces
//   [ ] SolidSyslog_Create returns a non-NULL handle
//   [ ] Two independently created loggers have different handles
//   [ ] Each logger sends through its own sender (independence at behaviour level)
//   [ ] SenderSpy can be substituted for any other SyslogSender without changing SolidSyslog
//   [ ] NullSender (internal default) does not crash when Send is called
//
// E — Exceptions
//   [x] SolidSyslog_Create returns NULL when alloc fails
//   [ ] NULL config to Create is handled without crash
//   [ ] SolidSyslog_Create with a NULL config returns NULL
//   [ ] SolidSyslog_Destroy with a NULL handle does not crash
//   [ ] SolidSyslog_Log on NULL handle does nothing, does not crash
//
// S — Simple scenario
//   [ ] A single Log call produces the fully validated RFC 5424 message
// clang-format on
