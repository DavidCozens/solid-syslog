#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogSender.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum
{
    SOLIDSYSLOG_MAX_MESSAGE_SIZE   = 128,
    SOLIDSYSLOG_MAX_TIMESTAMP_SIZE = 33
};

static inline uint8_t CombineFacilityAndSeverity(uint8_t facility, uint8_t severity);
static inline bool    FacilityIsValid(uint8_t facility);
static inline int     FormatMessage(char* buffer, size_t size, const struct SolidSyslogMessage* message, SolidSyslogClockFn clock);
static inline bool    CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFn clock);
static inline int     FormatCapturedTimestamp(char* buffer, const struct SolidSyslogTimestamp* ts);
static inline int     FormatCharacter(char* buffer, char value);
static inline int     FormatMicrosecond(char* buffer, uint32_t value);
static inline int     FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFn clock);
static inline int     FormatTwoDigit(char* buffer, uint8_t value);
static inline int     FormatNonZeroUtcOffset(char* buffer, int16_t offsetMinutes);
static inline int     FormatUtcOffset(char* buffer, int16_t offsetMinutes);
static inline int     FormatYear(char* buffer, uint16_t value);
static inline uint8_t MakePrival(const struct SolidSyslogMessage* message);
static inline bool    PrivalComponentsAreValid(uint8_t facility, uint8_t severity);
static inline bool    SeverityIsValid(uint8_t severity);

struct SolidSyslog
{
    struct SolidSyslogSender* sender;
    SolidSyslogFreeFn         free;
    SolidSyslogClockFn        clock;
};

struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config)
{
    struct SolidSyslog* instance = config->alloc(sizeof(struct SolidSyslog));
    if (instance != NULL)
    {
        instance->sender = config->sender;
        instance->free   = config->free;
        instance->clock  = config->clock;
    }
    return instance;
}

void SolidSyslog_Destroy(struct SolidSyslog* logger)
{
    logger->free(logger);
}

void SolidSyslog_Log(struct SolidSyslog* logger, const struct SolidSyslogMessage* message)
{
    char buffer[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    int  len = FormatMessage(buffer, sizeof(buffer), message, logger->clock);
    SolidSyslogSender_Send(logger->sender, buffer, (size_t) len);
}

static inline int FormatMessage(char* buffer, size_t size, const struct SolidSyslogMessage* message, SolidSyslogClockFn clock)
{
    uint8_t prival = MakePrival(message);
    char    timestamp[SOLIDSYSLOG_MAX_TIMESTAMP_SIZE];
    FormatTimestamp(timestamp, sizeof(timestamp), clock);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- snprintf is bounded; snprintf_s is not portable
    return snprintf(buffer, size, "<%d>1 %s TestHost TestApp 42 54 - hello world", prival, timestamp);
}

static inline int FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFn clock)
{
    (void) size;

    struct SolidSyslogTimestamp ts  = {0};
    int                        len = 0;

    if (CaptureTimestamp(&ts, clock))
    {
        len = FormatCapturedTimestamp(buffer, &ts);
    }
    else
    {
        len = FormatCharacter(buffer, '-');
    }

    return len;
}

static inline int FormatCapturedTimestamp(char* buffer, const struct SolidSyslogTimestamp* ts)
{
    int len = 0;

    len += FormatYear(buffer + len, ts->year);
    len += FormatCharacter(buffer + len, '-');
    len += FormatTwoDigit(buffer + len, ts->month);
    len += FormatCharacter(buffer + len, '-');
    len += FormatTwoDigit(buffer + len, ts->day);
    len += FormatCharacter(buffer + len, 'T');
    len += FormatTwoDigit(buffer + len, ts->hour);
    len += FormatCharacter(buffer + len, ':');
    len += FormatTwoDigit(buffer + len, ts->minute);
    len += FormatCharacter(buffer + len, ':');
    len += FormatTwoDigit(buffer + len, ts->second);
    len += FormatCharacter(buffer + len, '.');
    len += FormatMicrosecond(buffer + len, ts->microsecond);
    len += FormatUtcOffset(buffer + len, ts->utcOffsetMinutes);

    return len;
}

static inline bool CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFn clock)
{
    if (clock == NULL)
    {
        return false;
    }

    *ts = clock();
    return true;
}

static inline int FormatCharacter(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
}

static inline int FormatYear(char* buffer, uint16_t value)
{
    buffer[0] = (char)('0' + (value / 1000U) % 10U);
    buffer[1] = (char)('0' + (value / 100U) % 10U);
    buffer[2] = (char)('0' + (value / 10U) % 10U);
    buffer[3] = (char)('0' + value % 10U);
    buffer[4] = '\0';
    return 4;
}

static inline int FormatTwoDigit(char* buffer, uint8_t value)
{
    buffer[0] = (char)('0' + value / 10U);
    buffer[1] = (char)('0' + value % 10U);
    buffer[2] = '\0';
    return 2;
}

static inline int FormatMicrosecond(char* buffer, uint32_t value)
{
    buffer[0] = (char)('0' + (value / 100000U) % 10U);
    buffer[1] = (char)('0' + (value / 10000U) % 10U);
    buffer[2] = (char)('0' + (value / 1000U) % 10U);
    buffer[3] = (char)('0' + (value / 100U) % 10U);
    buffer[4] = (char)('0' + (value / 10U) % 10U);
    buffer[5] = (char)('0' + value % 10U);
    buffer[6] = '\0';
    return 6;
}

static inline int FormatUtcOffset(char* buffer, int16_t offsetMinutes)
{
    int len = 0;

    if (offsetMinutes == 0)
    {
        len = FormatCharacter(buffer, 'Z');
    }
    else
    {
        len = FormatNonZeroUtcOffset(buffer, offsetMinutes);
    }

    return len;
}

static inline int FormatNonZeroUtcOffset(char* buffer, int16_t offsetMinutes)
{
    char    sign            = (offsetMinutes > 0) ? '+' : '-';
    int16_t absoluteMinutes = (offsetMinutes > 0) ? offsetMinutes : (int16_t)-offsetMinutes;
    uint8_t hours           = (uint8_t)(absoluteMinutes / 60);
    uint8_t minutes         = (uint8_t)(absoluteMinutes % 60);
    int     len             = 0;

    len += FormatCharacter(buffer + len, sign);
    len += FormatTwoDigit(buffer + len, hours);
    len += FormatCharacter(buffer + len, ':');
    len += FormatTwoDigit(buffer + len, minutes);

    return len;
}

static inline uint8_t MakePrival(const struct SolidSyslogMessage* message)
{
    uint8_t f      = (uint8_t) message->facility;
    uint8_t s      = (uint8_t) message->severity;
    uint8_t prival = CombineFacilityAndSeverity(SOLIDSYSLOG_FACILITY_SYSLOG, SOLIDSYSLOG_SEVERITY_ERR);

    if (PrivalComponentsAreValid(f, s))
    {
        prival = CombineFacilityAndSeverity(f, s);
    }

    return prival;
}

static inline uint8_t CombineFacilityAndSeverity(uint8_t facility, uint8_t severity)
{
    return (uint8_t) ((facility * UINT8_C(8)) + severity);
}

static inline bool PrivalComponentsAreValid(uint8_t facility, uint8_t severity)
{
    return FacilityIsValid(facility) && SeverityIsValid(severity);
}

static inline bool FacilityIsValid(uint8_t facility)
{
    return facility <= SOLIDSYSLOG_FACILITY_LOCAL7;
}

static inline bool SeverityIsValid(uint8_t severity)
{
    return severity <= SOLIDSYSLOG_SEVERITY_DEBUG;
}
