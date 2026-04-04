#include "CppUTest/TestHarness.h"
#include "SolidSyslogExample.h"
#include "ClockFake.h"
#include "SocketSpy.h"

#include <cstring>
#include <getopt.h>

// clang-format off
TEST_GROUP(SolidSyslogExample)
{
    void setup() override
    {
        SocketSpy_Reset();
        ClockFake_Reset();
        ClockFake_SetTime(1743768600, 0);
        optind = 1;
    }

    int Run(int argc, char* argv[])
    {
        return SolidSyslogExample_Run(argc, argv);
    }

    int RunWithNoArgs()
    {
        char  arg0[] = "SolidSyslogExample";
        char* argv[] = {arg0, nullptr};
        return Run(1, argv);
    }
};

// clang-format on

TEST(SolidSyslogExample, InvalidOptionReturnsOne)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--invalid";
    char* argv[] = {arg0, arg1, nullptr};
    LONGS_EQUAL(1, Run(2, argv));
}

TEST(SolidSyslogExample, RunWithNoArgsReturnsZero)
{
    LONGS_EQUAL(0, RunWithNoArgs());
}

TEST(SolidSyslogExample, RunSendsOneMessage)
{
    RunWithNoArgs();
    LONGS_EQUAL(1, SocketSpy_SendtoCallCount());
}

TEST(SolidSyslogExample, DefaultMessageContainsLocal0InfoPrival)
{
    RunWithNoArgs();
    STRNCMP_EQUAL("<134>", SocketSpy_LastBufAsString(), 5);
}

TEST(SolidSyslogExample, FacilityFlagAppearsInPrival)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--facility";
    char  arg2[] = "0";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    Run(3, argv);
    STRNCMP_EQUAL("<6>", SocketSpy_LastBufAsString(), 3);
}

TEST(SolidSyslogExample, SeverityFlagAppearsInPrival)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--severity";
    char  arg2[] = "0";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    Run(3, argv);
    STRNCMP_EQUAL("<128>", SocketSpy_LastBufAsString(), 5);
}

TEST(SolidSyslogExample, ResolvesConfiguredHost)
{
    RunWithNoArgs();
    STRCMP_EQUAL("syslog-ng", SocketSpy_LastGetAddrInfoHostname());
}

TEST(SolidSyslogExample, SendsToConfiguredPort)
{
    RunWithNoArgs();
    LONGS_EQUAL(5514, SocketSpy_LastPort());
}

TEST(SolidSyslogExample, AppNameDerivedFromArgv0)
{
    char  arg0[] = "/usr/bin/MyApp";
    char* argv[] = {arg0, nullptr};
    Run(1, argv);
    const char* msg = SocketSpy_LastBufAsString();
    CHECK(strstr(msg, "MyApp") != nullptr);
}

TEST(SolidSyslogExample, SocketCreatedWithUdpDgram)
{
    RunWithNoArgs();
    LONGS_EQUAL(AF_INET, SocketSpy_SocketDomain());
    LONGS_EQUAL(SOCK_DGRAM, SocketSpy_SocketType());
}

TEST(SolidSyslogExample, SocketClosedAfterRun)
{
    RunWithNoArgs();
    LONGS_EQUAL(1, SocketSpy_CloseCallCount());
}

TEST(SolidSyslogExample, MsgIdFlagAppearsInMessage)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--msgid";
    char  arg2[] = "ID47";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    Run(3, argv);
    CHECK(std::strstr(SocketSpy_LastBufAsString(), "ID47") != nullptr);
}

TEST(SolidSyslogExample, CountFlagSendsMultipleMessages)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--count";
    char  arg2[] = "3";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    Run(3, argv);
    LONGS_EQUAL(3, SocketSpy_SendtoCallCount());
}

TEST(SolidSyslogExample, MessageFlagAppearsInMessage)
{
    char  arg0[] = "SolidSyslogExample";
    char  arg1[] = "--message";
    char  arg2[] = "system started";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    Run(3, argv);
    CHECK(std::strstr(SocketSpy_LastBufAsString(), "system started") != nullptr);
}
