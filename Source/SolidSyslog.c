#include "SolidSyslog.h"
#include "SolidSyslogBuffer.h"
#include "SolidSyslogConfig.h"

#include <stdbool.h>
#include <stdint.h>

enum
{
    SOLIDSYSLOG_MAX_APP_NAME_SIZE  = 49,
    SOLIDSYSLOG_MAX_HOSTNAME_SIZE  = 256,
    SOLIDSYSLOG_MAX_MSGID_SIZE     = 33,
    SOLIDSYSLOG_MAX_PROCID_SIZE    = 129,
    SOLIDSYSLOG_MAX_TIMESTAMP_SIZE = 33
};

static inline int16_t AbsoluteInt16(int16_t value);
static inline bool    CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFunction clock);
static inline uint8_t CombineFacilityAndSeverity(uint8_t facility, uint8_t severity);
static inline bool    FacilityIsValid(uint8_t facility);
static inline size_t  FormatAppName(char* buffer, SolidSyslogStringFunction getAppName);
static inline size_t  FormatAsHours(char* buffer, int16_t absoluteMinutes);
static inline size_t  FormatAsMinutes(char* buffer, int16_t absoluteMinutes);
static inline size_t  FormatBoundedString(char* buffer, const char* source, size_t bufferSize);
static inline size_t  FormatCapturedTimestamp(char* buffer, const struct SolidSyslogTimestamp* ts);
static inline size_t  FormatCharacter(char* buffer, char value);
static inline size_t  FormatHostname(char* buffer, SolidSyslogStringFunction getHostname);
static inline size_t  FormatMessage(const struct SolidSyslog* self, char* buffer, size_t size, const struct SolidSyslogMessage* message);
static inline size_t  FormatMicrosecond(char* buffer, uint32_t value);
static inline size_t  FormatMsg(char* buffer, const char* msg, size_t remaining);
static inline size_t  FormatMsgId(char* buffer, const char* messageId);
static inline size_t  FormatNilvalue(char* buffer);
static inline size_t  FormatNonZeroUtcOffset(char* buffer, int16_t offsetMinutes);
static inline size_t  FormatPrival(char* buffer, uint8_t prival);
static inline size_t  FormatProcId(char* buffer, SolidSyslogStringFunction getProcId);
static inline size_t  FormatSign(char* buffer, int16_t value);
static inline size_t  FormatSpace(char* buffer);
static inline size_t  FormatStructuredData(char* buffer);
static inline size_t  FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFunction clock);
static inline size_t  FormatTwoDigit(char* buffer, uint8_t value);
static inline size_t  FormatUtcOffset(char* buffer, int16_t offsetMinutes);
static inline size_t  FormatVersion(char* buffer);
static inline size_t  FormatYear(char* buffer, uint16_t value);
static inline uint8_t MakePrival(const struct SolidSyslogMessage* message);
static inline bool    PrivalComponentsAreValid(uint8_t facility, uint8_t severity);
static inline bool    SeverityIsValid(uint8_t severity);
static inline bool    StringIsValid(const char* value);
static inline bool    TimestampIsValid(const struct SolidSyslogTimestamp* ts);

struct SolidSyslog
{
    struct SolidSyslogBuffer* buffer;
    SolidSyslogFreeFunction   free;
    SolidSyslogClockFunction  clock;
    SolidSyslogStringFunction getHostname;
    SolidSyslogStringFunction getAppName;
    SolidSyslogStringFunction getProcId;
};

struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config)
{
    struct SolidSyslog* instance = config->alloc(sizeof(struct SolidSyslog));
    if (instance != NULL)
    {
        instance->buffer      = config->buffer;
        instance->free        = config->free;
        instance->clock       = config->clock;
        instance->getHostname = config->getHostname;
        instance->getAppName  = config->getAppName;
        instance->getProcId   = config->getProcId;
    }
    return instance;
}

void SolidSyslog_Destroy(struct SolidSyslog* logger)
{
    logger->free(logger);
}

bool SolidSyslog_Service(struct SolidSyslog* logger)
{
    return SolidSyslogBuffer_Read(logger->buffer);
}

void SolidSyslog_Log(struct SolidSyslog* logger, const struct SolidSyslogMessage* message)
{
    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    size_t len = FormatMessage(logger, buf, sizeof(buf), message);
    SolidSyslogBuffer_Write(logger->buffer, buf, len);
}

static inline size_t FormatMessage(const struct SolidSyslog* self, char* buffer, size_t size, const struct SolidSyslogMessage* message)
{
    size_t len = 0;

    len += FormatPrival(buffer + len, MakePrival(message));
    len += FormatVersion(buffer + len);
    len += FormatSpace(buffer + len);
    len += FormatTimestamp(buffer + len, SOLIDSYSLOG_MAX_TIMESTAMP_SIZE, self->clock);
    len += FormatSpace(buffer + len);
    len += FormatHostname(buffer + len, self->getHostname);
    len += FormatSpace(buffer + len);
    len += FormatAppName(buffer + len, self->getAppName);
    len += FormatSpace(buffer + len);
    len += FormatProcId(buffer + len, self->getProcId);
    len += FormatSpace(buffer + len);
    len += FormatMsgId(buffer + len, message->messageId);
    len += FormatSpace(buffer + len);
    len += FormatStructuredData(buffer + len);
    len += FormatMsg(buffer + len, message->msg, size - len);

    return len;
}

static inline size_t FormatPrival(char* buffer, uint8_t prival)
{
    size_t len = 0;

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

static inline size_t FormatCharacter(char* buffer, char value)
{
    buffer[0] = value;
    buffer[1] = '\0';
    return 1;
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

static inline size_t FormatVersion(char* buffer)
{
    return FormatCharacter(buffer, '1');
}

static inline size_t FormatSpace(char* buffer)
{
    return FormatCharacter(buffer, ' ');
}

static inline size_t FormatTimestamp(char* buffer, size_t size, SolidSyslogClockFunction clock)
{
    (void) size;

    struct SolidSyslogTimestamp ts  = {0};
    size_t                      len = 0;

    if (CaptureTimestamp(&ts, clock))
    {
        len = FormatCapturedTimestamp(buffer, &ts);
    }
    else
    {
        len = FormatNilvalue(buffer);
    }

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

static inline size_t FormatCapturedTimestamp(char* buffer, const struct SolidSyslogTimestamp* ts)
{
    size_t len = 0;

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

static inline size_t FormatYear(char* buffer, uint16_t value)
{
    buffer[0] = (char) ('0' + ((value / 1000U) % 10U));
    buffer[1] = (char) ('0' + ((value / 100U) % 10U));
    buffer[2] = (char) ('0' + ((value / 10U) % 10U));
    buffer[3] = (char) ('0' + (value % 10U));
    buffer[4] = '\0';
    return 4;
}

static inline size_t FormatTwoDigit(char* buffer, uint8_t value)
{
    buffer[0] = (char) ('0' + (value / 10U));
    buffer[1] = (char) ('0' + (value % 10U));
    buffer[2] = '\0';
    return 2;
}

static inline size_t FormatMicrosecond(char* buffer, uint32_t value)
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

static inline size_t FormatUtcOffset(char* buffer, int16_t offsetMinutes)
{
    size_t len = 0;

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

static inline size_t FormatNonZeroUtcOffset(char* buffer, int16_t offsetMinutes)
{
    int16_t absoluteMinutes = AbsoluteInt16(offsetMinutes);
    size_t  len             = 0;

    len += FormatSign(buffer + len, offsetMinutes);
    len += FormatAsHours(buffer + len, absoluteMinutes);
    len += FormatCharacter(buffer + len, ':');
    len += FormatAsMinutes(buffer + len, absoluteMinutes);

    return len;
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

static inline size_t FormatSign(char* buffer, int16_t value)
{
    return FormatCharacter(buffer, (value > 0) ? '+' : '-');
}

static inline size_t FormatAsHours(char* buffer, int16_t absoluteMinutes)
{
    return FormatTwoDigit(buffer, (uint8_t) (absoluteMinutes / 60));
}

static inline size_t FormatAsMinutes(char* buffer, int16_t absoluteMinutes)
{
    return FormatTwoDigit(buffer, (uint8_t) (absoluteMinutes % 60));
}

static inline size_t FormatNilvalue(char* buffer)
{
    return FormatCharacter(buffer, '-');
}

static inline size_t FormatHostname(char* buffer, SolidSyslogStringFunction getHostname)
{
    size_t len = 0;

    if (getHostname != NULL)
    {
        len = getHostname(buffer, SOLIDSYSLOG_MAX_HOSTNAME_SIZE);
    }

    if (len == 0)
    {
        len = FormatNilvalue(buffer);
    }

    return len;
}

static inline size_t FormatAppName(char* buffer, SolidSyslogStringFunction getAppName)
{
    size_t len = 0;

    if (getAppName != NULL)
    {
        len = getAppName(buffer, SOLIDSYSLOG_MAX_APP_NAME_SIZE);
    }

    if (len == 0)
    {
        len = FormatNilvalue(buffer);
    }

    return len;
}

static inline size_t FormatProcId(char* buffer, SolidSyslogStringFunction getProcId)
{
    size_t len = 0;

    if (getProcId != NULL)
    {
        len = getProcId(buffer, SOLIDSYSLOG_MAX_PROCID_SIZE);
    }

    if (len == 0)
    {
        len = FormatNilvalue(buffer);
    }

    return len;
}

static inline size_t FormatMsgId(char* buffer, const char* messageId)
{
    size_t len = 0;

    if (StringIsValid(messageId))
    {
        len = FormatBoundedString(buffer, messageId, SOLIDSYSLOG_MAX_MSGID_SIZE);
    }

    if (len == 0)
    {
        len = FormatNilvalue(buffer);
    }

    return len;
}

static inline bool StringIsValid(const char* value)
{
    return (value != NULL) && (value[0] != '\0');
}

static inline size_t FormatBoundedString(char* buffer, const char* source, size_t bufferSize)
{
    size_t maxLength = (bufferSize > 0) ? bufferSize - 1 : 0;
    size_t len       = 0;
    while ((len < maxLength) && (source[len] != '\0'))
    {
        buffer[len] = source[len];
        len++;
    }
    buffer[len] = '\0';
    return len;
}

static inline size_t FormatStructuredData(char* buffer)
{
    return FormatNilvalue(buffer);
}

static inline size_t FormatMsg(char* buffer, const char* msg, size_t remaining)
{
    size_t len = 0;

    if (StringIsValid(msg))
    {
        len += FormatSpace(buffer + len);
        len += FormatBoundedString(buffer + len, msg, remaining - len);
    }

    return len;
}
