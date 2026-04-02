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
static inline int     FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFn clock);
static inline int     FormatUtcOffset(char* buffer, size_t size, int16_t offsetMinutes);
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
    if (clock == NULL)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
        return snprintf(buffer, size, "-");
    }

    struct SolidSyslogTimestamp ts = clock();

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    int len = snprintf(buffer, size, "%04u-%02u-%02uT%02u:%02u:%02u.%06u",
                       ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.microsecond);

    len += FormatUtcOffset(buffer + len, size - (size_t) len, ts.utcOffsetMinutes);

    return len;
}

static inline int FormatUtcOffset(char* buffer, size_t size, int16_t offsetMinutes)
{
    if (offsetMinutes == 0)
    {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
        return snprintf(buffer, size, "Z");
    }

    int hours   = offsetMinutes / 60;
    int minutes = offsetMinutes % 60;
    if (minutes < 0) { minutes = -minutes; }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    return snprintf(buffer, size, "%+03d:%02d", hours, minutes);
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
