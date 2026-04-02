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
static inline int     FormatMicrosecond(char* buffer, uint32_t value);
static inline int     FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFn clock);
static inline int     FormatTwoDigit(char* buffer, uint8_t value);
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

    if (clock == NULL)
    {
        buffer[0] = '-';
        buffer[1] = '\0';
        return 1;
    }

    struct SolidSyslogTimestamp ts = clock();
    char* p = buffer;

    p += FormatYear(p, ts.year);
    *p++ = '-';
    p += FormatTwoDigit(p, ts.month);
    *p++ = '-';
    p += FormatTwoDigit(p, ts.day);
    *p++ = 'T';
    p += FormatTwoDigit(p, ts.hour);
    *p++ = ':';
    p += FormatTwoDigit(p, ts.minute);
    *p++ = ':';
    p += FormatTwoDigit(p, ts.second);
    *p++ = '.';
    p += FormatMicrosecond(p, ts.microsecond);
    p += FormatUtcOffset(p, ts.utcOffsetMinutes);
    *p = '\0';

    return (int)(p - buffer);
}

static inline int FormatYear(char* buffer, uint16_t value)
{
    buffer[0] = (char)('0' + (value / 1000U) % 10U);
    buffer[1] = (char)('0' + (value / 100U) % 10U);
    buffer[2] = (char)('0' + (value / 10U) % 10U);
    buffer[3] = (char)('0' + value % 10U);
    return 4;
}

static inline int FormatTwoDigit(char* buffer, uint8_t value)
{
    buffer[0] = (char)('0' + value / 10U);
    buffer[1] = (char)('0' + value % 10U);
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
    return 6;
}

static inline int FormatUtcOffset(char* buffer, int16_t offsetMinutes)
{
    if (offsetMinutes == 0)
    {
        buffer[0] = 'Z';
        return 1;
    }

    int16_t hours   = offsetMinutes / 60;
    int16_t minutes = offsetMinutes % 60;
    if (minutes < 0) { minutes = (int16_t)-minutes; }

    buffer[0] = (offsetMinutes > 0) ? '+' : '-';
    if (hours < 0) { hours = (int16_t)-hours; }
    buffer[1] = (char)('0' + hours / 10);
    buffer[2] = (char)('0' + hours % 10);
    buffer[3] = ':';
    buffer[4] = (char)('0' + minutes / 10);
    buffer[5] = (char)('0' + minutes % 10);
    return 6;
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
