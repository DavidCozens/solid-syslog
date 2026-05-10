#include "CppUTest/TestHarness.h"

#include "SolidSyslogConfig.h"
#include "SolidSyslogError.h"
#include "SolidSyslogErrorMessages.h"
#include "SolidSyslogPrival.h"

static int                       handlerCallCount;
static enum SolidSyslog_Severity capturedSeverity;
static const char*               capturedMessage;
static void*                     capturedContext;

static void TestErrorHandler(void* context, enum SolidSyslog_Severity severity, const char* message)
{
    handlerCallCount++;
    capturedSeverity = severity;
    capturedMessage  = message;
    capturedContext  = context;
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- macros preserve __FILE__/__LINE__ in test failure output
#define CHECK_HANDLER_INVOKED_ONCE() LONGS_EQUAL(1, handlerCallCount)
#define CHECK_HANDLER_NOT_INVOKED() LONGS_EQUAL(0, handlerCallCount)
#define CHECK_EXPECTED_SEVERITY(expected) LONGS_EQUAL((expected), capturedSeverity)
#define CHECK_EXPECTED_MESSAGE(expected) STRCMP_EQUAL((expected), capturedMessage)
#define CHECK_EXPECTED_CONTEXT(expected) POINTERS_EQUAL((expected), capturedContext)

// NOLINTEND(cppcoreguidelines-macro-usage)

// clang-format off
TEST_GROUP(SolidSyslogError)
{
    int sentinel = 0;

    void setup() override
    {
        handlerCallCount = 0;
        capturedSeverity = SOLIDSYSLOG_SEVERITY_DEBUG;
        capturedMessage  = nullptr;
        capturedContext  = nullptr;
    }

    void teardown() override
    {
        SolidSyslog_SetErrorHandler(nullptr, nullptr);
    }

    void installHandler()
    {
        SolidSyslog_SetErrorHandler(TestErrorHandler, &sentinel);
    }
};

// clang-format on

TEST(SolidSyslogError, ErrorWithDefaultHandlerDoesNotCrash)
{
    SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERR, "test message");
}

TEST(SolidSyslogError, InstalledHandlerReceivesSeverityMessageAndContext)
{
    installHandler();
    SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_WARNING, "warning message");

    CHECK_HANDLER_INVOKED_ONCE();
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_WARNING);
    CHECK_EXPECTED_MESSAGE("warning message");
    CHECK_EXPECTED_CONTEXT(&sentinel);
}

TEST(SolidSyslogError, SetErrorHandlerWithNullHandlerRestoresDefault)
{
    installHandler();

    SolidSyslog_SetErrorHandler(nullptr, &sentinel);
    SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERR, "should not be observed");

    CHECK_HANDLER_NOT_INVOKED();
}

TEST(SolidSyslogError, SolidSyslogCreateWithNullConfigReportsError)
{
    installHandler();

    SolidSyslog_Create(nullptr);

    CHECK_HANDLER_INVOKED_ONCE();
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_CREATE_NULL_CONFIG);
}
