#include "CppUTest/TestHarness.h"
#include "SolidSyslog.h"
#include "SpySender.h"

// clang-format off
TEST_GROUP(SolidSyslog)
{
    SolidSyslog_Config config;
    SolidSyslog *logger;

    void setup() override
    {
        SpySender_Reset();
        config = {SpySender_GetSender()};
        logger = SolidSyslog_Create(&config);
    }

    void teardown() override
    {
        SolidSyslog_Destroy(logger);
    }
};

// clang-format on

TEST(SolidSyslog, CreateDestroyWorksWithoutCrashing)
{
}

TEST(SolidSyslog, NoMessagesAreSentWhenLogIsNotCalled)
{
    LONGS_EQUAL(0, SpySender_CallCount());
}

TEST(SolidSyslog, SingleLogCallResultsInOneSend)
{
    SolidSyslog_Log(logger);
    LONGS_EQUAL(1, SpySender_CallCount());
}

// clang-format off
// Test list — S1.1: Walking Skeleton
//
// Z — Zero
//   [x] No messages are sent when log is not called
//
// O — One
//   [x] A single SolidSyslog_Log(logger) call results in exactly one Send on the spy
//   [ ] The sent buffer starts with '<'
//   [ ] The sent buffer contains version '1' in the correct position
//
// B — Boundaries
//   [ ] SolidSyslog_Log called twice results in Send called twice
//
// I — Interfaces
//   [ ] SpySender can be substituted for any other SyslogSender without changing SolidSyslog
//   [ ] NullSender (internal default) does not crash when Send is called
//
// E — Exceptions
//   [ ] NULL config to Create is handled without crash
//   [ ] SolidSyslog_Create with a NULL config returns NULL
//   [ ] SolidSyslog_Log on NULL handle does nothing, does not crash
//
// S — Simple scenario
//   [ ] A single Log call produces a string matching the RFC 5424 structure:
//       <PRIVAL>1 TIMESTAMP HOSTNAME APP-NAME PROCID MSGID MSG
//       with all fields present (NILVALUE '-' acceptable for all)
// clang-format on
