#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogSender.h"

#include <stdbool.h>
#include <stdint.h>

enum
{
    SOLIDSYSLOG_MAX_HOSTNAME_SIZE  = 256,
    SOLIDSYSLOG_MAX_MESSAGE_SIZE   = 128,
    SOLIDSYSLOG_MAX_TIMESTAMP_SIZE = 33
};

static inline uint8_t CombineFacilityAndSeverity(uint8_t facility, uint8_t severity);
static inline bool    CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFunction clock);
static inline bool    FacilityIsValid(uint8_t facility);
static inline int     FormatMessage(const struct SolidSyslog* self, char* buffer, size_t size, const struct SolidSyslogMessage* message);
static inline bool    TimestampIsValid(const struct SolidSyslogTimestamp* ts);
static inline int     FormatCapturedTimestamp(char* buffer, const struct SolidSyslogTimestamp* ts);
static inline int     FormatCharacter(char* buffer, char value);
static inline int     FormatHostname(char* buffer, SolidSyslogStringFunction getHostname);
static inline int     FormatMicrosecond(char* buffer, uint32_t value);
static inline int     FormatPrival(char* buffer, uint8_t prival);
static inline int     FormatVersion(char* buffer);
static inline int     FormatSpace(char* buffer);
static inline int     FormatString(char* buffer, const char* source);
static inline int     FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFunction clock);
static inline int     FormatTwoDigit(char* buffer, uint8_t value);
static inline int16_t AbsoluteInt16(int16_t value);
static inline int     FormatAsHours(char* buffer, int16_t absoluteMinutes);
static inline int     FormatAsMinutes(char* buffer, int16_t absoluteMinutes);
static inline int     FormatNonZeroUtcOffset(char* buffer, int16_t offsetMinutes);
static inline int     FormatSign(char* buffer, int16_t value);
static inline int     FormatUtcOffset(char* buffer, int16_t offsetMinutes);
static inline int     FormatYear(char* buffer, uint16_t value);
static inline uint8_t MakePrival(const struct SolidSyslogMessage* message);
static inline bool    PrivalComponentsAreValid(uint8_t facility, uint8_t severity);
static inline bool    SeverityIsValid(uint8_t severity);

struct SolidSyslog
{
    struct SolidSyslogSender* sender;
    SolidSyslogFreeFunction   free;
    SolidSyslogClockFunction  clock;
    SolidSyslogStringFunction getHostname;
};

struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config)
{
    struct SolidSyslog* instance = config->alloc(sizeof(struct SolidSyslog));
    if (instance != NULL)
    {
        instance->sender      = config->sender;
        instance->free        = config->free;
        instance->clock       = config->clock;
        instance->getHostname = config->getHostname;
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
    int  len = FormatMessage(logger, buffer, sizeof(buffer), message);
    SolidSyslogSender_Send(logger->sender, buffer, (size_t) len);
}

static inline int FormatMessage(const struct SolidSyslog* self, char* buffer, size_t size, const struct SolidSyslogMessage* message)
{
    (void) size;
    int len = 0;

    len += FormatPrival(buffer + len, MakePrival(message));
    len += FormatVersion(buffer + len);
    len += FormatSpace(buffer + len);
    len += FormatTimestamp(buffer + len, SOLIDSYSLOG_MAX_TIMESTAMP_SIZE, self->clock);
    len += FormatSpace(buffer + len);
    len += FormatHostname(buffer + len, self->getHostname);
    len += FormatString(buffer + len, " TestApp 42 54 - hello world");

    return len;
}

static inline int FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFunction clock)
{
    (void) size;

    struct SolidSyslogTimestamp ts  = {0};
    int                         len = 0;

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

static inline bool CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFunction clock)
{
    bool captured = false;

    if (clock != NULL)
    {
        *ts      = clock();
        captured = TimestampIsValid(ts);
    }

    return captured;
}

static inline bool TimestampIsValid(const struct SolidSyslogTimestamp* ts)
{
    bool valid = true;

    valid = valid && (ts->month >= 1U) && (ts->month <= 12U);
    valid = valid && (ts->day >= 1U) && (ts->day <= 31U);
    valid = valid && (ts->hour <= 23U);
    valid = valid && (ts->minute <= 59U);
    valid = valid && (ts->second <= 59U);
    valid = valid && (ts->microsecond <= 999999U);
    valid = valid && (ts->utcOffsetMinutes >= -720) && (ts->utcOffsetMinutes <= 840);

    return valid;
}

static inline int FormatCharacter(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
}

static inline int FormatHostname(char* buffer, SolidSyslogStringFunction getHostname)
{
    int len = 0;

    if (getHostname != NULL)
    {
        len = getHostname(buffer, SOLIDSYSLOG_MAX_HOSTNAME_SIZE);
    }
    else
    {
        len = FormatCharacter(buffer, '-');
    }

    return len;
}

static inline int FormatPrival(char* buffer, uint8_t prival)
{
    int len = 0;

    len += FormatCharacter(buffer + len, '<');
    if (prival >= 100U)
    {
        len += FormatCharacter(buffer + len, (char) ('0' + (prival / 100U)));
    }
    if (prival >= 10U)
    {
        len += FormatCharacter(buffer + len, (char) ('0' + ((prival / 10U) % 10U)));
    }
    len += FormatCharacter(buffer + len, (char) ('0' + (prival % 10U)));
    len += FormatCharacter(buffer + len, '>');

    return len;
}

static inline int FormatSpace(char* buffer)
{
    return FormatCharacter(buffer, ' ');
}

static inline int FormatVersion(char* buffer)
{
    return FormatCharacter(buffer, '1');
}

static inline int FormatString(char* buffer, const char* source)
{
    int len = 0;
    while (source[len] != '\0')
    {
        buffer[len] = source[len];
        len++;
    }
    buffer[len] = '\0';
    return len;
}

static inline int FormatYear(char* buffer, uint16_t value)
{
    buffer[0] = (char) ('0' + ((value / 1000U) % 10U));
    buffer[1] = (char) ('0' + ((value / 100U) % 10U));
    buffer[2] = (char) ('0' + ((value / 10U) % 10U));
    buffer[3] = (char) ('0' + (value % 10U));
    buffer[4] = '\0';
    return 4;
}

static inline int FormatTwoDigit(char* buffer, uint8_t value)
{
    buffer[0] = (char) ('0' + (value / 10U));
    buffer[1] = (char) ('0' + (value % 10U));
    buffer[2] = '\0';
    return 2;
}

static inline int FormatMicrosecond(char* buffer, uint32_t value)
{
    buffer[0] = (char) ('0' + ((value / 100000U) % 10U));
    buffer[1] = (char) ('0' + ((value / 10000U) % 10U));
    buffer[2] = (char) ('0' + ((value / 1000U) % 10U));
    buffer[3] = (char) ('0' + ((value / 100U) % 10U));
    buffer[4] = (char) ('0' + ((value / 10U) % 10U));
    buffer[5] = (char) ('0' + (value % 10U));
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
    int16_t absoluteMinutes = AbsoluteInt16(offsetMinutes);
    int     len             = 0;

    len += FormatSign(buffer + len, offsetMinutes);
    len += FormatAsHours(buffer + len, absoluteMinutes);
    len += FormatCharacter(buffer + len, ':');
    len += FormatAsMinutes(buffer + len, absoluteMinutes);

    return len;
}

static inline int FormatAsHours(char* buffer, int16_t absoluteMinutes)
{
    return FormatTwoDigit(buffer, (uint8_t) (absoluteMinutes / 60));
}

static inline int FormatAsMinutes(char* buffer, int16_t absoluteMinutes)
{
    return FormatTwoDigit(buffer, (uint8_t) (absoluteMinutes % 60));
}

static inline int16_t AbsoluteInt16(int16_t value)
{
    int16_t result = value;

    if (value < 0)
    {
        result = (int16_t) (-value);
    }

    return result;
}

static inline int FormatSign(char* buffer, int16_t value)
{
    return FormatCharacter(buffer, (value > 0) ? '+' : '-');
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
