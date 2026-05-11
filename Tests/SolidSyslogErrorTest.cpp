#include "CppUTest/TestHarness.h"

#include "BufferFake.h"
#include "SenderFake.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogError.h"
#include "SolidSyslogErrorMessages.h"
#include "SolidSyslogNullStore.h"
#include "SolidSyslogPrival.h"
#include "TestUtils.h"

using namespace CososoTesting; // NOLINT(google-build-using-namespace) -- test-file scope only; brings NEVER/ONCE/TWICE/THRICE into scope for the CALLED_*
                               // macros

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

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_WARNING);
    CHECK_EXPECTED_MESSAGE("warning message");
    CHECK_EXPECTED_CONTEXT(&sentinel);
}

TEST(SolidSyslogError, SetErrorHandlerWithNullHandlerRestoresDefault)
{
    installHandler();

    SolidSyslog_SetErrorHandler(nullptr, &sentinel);
    SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERR, "should not be observed");

    CALLED_FUNCTION(handler, NEVER);
}

TEST(SolidSyslogError, SolidSyslogCreateWithNullConfigReportsError)
{
    installHandler();

    SolidSyslog_Create(nullptr);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_CREATE_NULL_CONFIG);
}

// clang-format off
TEST_GROUP(SolidSyslogLifecycle)
{
    SolidSyslogMessage message{};
    SolidSyslogBuffer* buffer = nullptr;
    SolidSyslogSender* sender = nullptr;
    SolidSyslogStore*  store  = nullptr;
    int                sentinel = 0;

    void setup() override
    {
        handlerCallCount = 0;
        capturedSeverity = SOLIDSYSLOG_SEVERITY_DEBUG;
        capturedMessage  = nullptr;
        capturedContext  = nullptr;
        SolidSyslog_Destroy();
        message = {SOLIDSYSLOG_FACILITY_LOCAL0, SOLIDSYSLOG_SEVERITY_INFO, nullptr, nullptr};
        sender  = SenderFake_Create();
        buffer  = BufferFake_Create();
        store   = SolidSyslogNullStore_Create();
    }

    void teardown() override
    {
        SolidSyslog_Destroy();
        SolidSyslog_SetErrorHandler(nullptr, nullptr);
        SolidSyslogNullStore_Destroy();
        BufferFake_Destroy();
        SenderFake_Destroy(sender);
    }

    void installHandler()
    {
        SolidSyslog_SetErrorHandler(TestErrorHandler, &sentinel);
    }

    [[nodiscard]] SolidSyslogConfig validConfig() const
    {
        return {buffer, sender, nullptr, nullptr, nullptr, nullptr, store, nullptr, 0};
    }
};

// clang-format on

TEST(SolidSyslogLifecycle, ServiceBeforeCreateDoesNotCrash)
{
    SolidSyslog_Service();
}

TEST(SolidSyslogLifecycle, LogBeforeCreateDoesNotCrash)
{
    SolidSyslog_Log(&message);
}

TEST(SolidSyslogLifecycle, LogBeforeCreateReportsNilBufferUsedOnce)
{
    installHandler();

    SolidSyslog_Log(&message);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_NIL_BUFFER_USED);
}

TEST(SolidSyslogLifecycle, RepeatedLogBeforeCreateReportsNilBufferUsedOnlyOnce)
{
    installHandler();

    SolidSyslog_Log(&message);
    SolidSyslog_Log(&message);
    SolidSyslog_Log(&message);

    CALLED_FUNCTION(handler, ONCE);
}

TEST(SolidSyslogLifecycle, DestroyReArmsNilBufferReporter)
{
    SolidSyslog_Log(&message);
    installHandler();

    SolidSyslog_Destroy();
    SolidSyslog_Log(&message);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_NIL_BUFFER_USED);
}

TEST(SolidSyslogLifecycle, LogWithNullMessageReportsError)
{
    installHandler();

    SolidSyslog_Log(nullptr);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_LOG_NULL_MESSAGE);
}

TEST(SolidSyslogLifecycle, CreateWithNullBufferReportsError)
{
    installHandler();
    SolidSyslogConfig config = validConfig();
    config.buffer            = nullptr;

    SolidSyslog_Create(&config);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_CREATE_NULL_BUFFER);
}

TEST(SolidSyslogLifecycle, CreateWithNullSenderReportsError)
{
    installHandler();
    SolidSyslogConfig config = validConfig();
    config.sender            = nullptr;

    SolidSyslog_Create(&config);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_CREATE_NULL_SENDER);
}

TEST(SolidSyslogLifecycle, CreateWithNullStoreReportsError)
{
    installHandler();
    SolidSyslogConfig config = validConfig();
    config.store             = nullptr;

    SolidSyslog_Create(&config);

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_CREATE_NULL_STORE);
}

TEST(SolidSyslogLifecycle, ServiceWithNilSenderReportsNilSenderUsed)
{
    SolidSyslogConfig config = validConfig();
    config.sender            = nullptr;
    SolidSyslog_Create(&config);
    SolidSyslog_Log(&message);
    installHandler();

    SolidSyslog_Service();

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_SEVERITY(SOLIDSYSLOG_SEVERITY_ERR);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_NIL_SENDER_USED);
}

TEST(SolidSyslogLifecycle, RepeatedServiceWithNilSenderReportsNilSenderUsedOnlyOnce)
{
    SolidSyslogConfig config = validConfig();
    config.sender            = nullptr;
    SolidSyslog_Create(&config);
    SolidSyslog_Log(&message);
    SolidSyslog_Service();
    installHandler();

    SolidSyslog_Log(&message);
    SolidSyslog_Service();
    SolidSyslog_Log(&message);
    SolidSyslog_Service();

    CALLED_FUNCTION(handler, NEVER);
}

TEST(SolidSyslogLifecycle, ServiceWithNilStoreDrainsThroughToRealSender)
{
    SolidSyslogConfig config = validConfig();
    config.store             = nullptr;
    SolidSyslog_Create(&config);
    SolidSyslog_Log(&message);

    SolidSyslog_Service();

    CALLED_FAKE_ON(SenderFake_Send, sender, ONCE);
}

TEST(SolidSyslogLifecycle, DestroyReArmsNilSenderReporter)
{
    SolidSyslogConfig config = validConfig();
    config.sender            = nullptr;
    SolidSyslog_Create(&config);
    SolidSyslog_Log(&message);
    SolidSyslog_Service();

    SolidSyslog_Destroy();
    SolidSyslog_Create(&config);
    SolidSyslog_Log(&message);
    installHandler();
    SolidSyslog_Service();

    CALLED_FUNCTION(handler, ONCE);
    CHECK_EXPECTED_MESSAGE(SOLIDSYSLOG_ERROR_MSG_NIL_SENDER_USED);
}
