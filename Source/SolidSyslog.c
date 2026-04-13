#include "SolidSyslog.h"
#include "SolidSyslogBuffer.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogStore.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogMacros.h"
#include "SolidSyslogSender.h"
#include "SolidSyslogStructuredData.h"

#include <stdbool.h>
#include <stdint.h>

enum
{
    SOLIDSYSLOG_MAX_APP_NAME_SIZE   = 49,
    SOLIDSYSLOG_MAX_HOSTNAME_SIZE   = 256,
    SOLIDSYSLOG_MAX_MSGID_SIZE      = 33,
    SOLIDSYSLOG_MAX_PROCESS_ID_SIZE = 129
};

static inline int16_t AbsoluteInt16(int16_t value);
static inline bool    CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFunction clock);
static inline uint8_t CombineFacilityAndSeverity(uint8_t facility, uint8_t severity);
static inline bool    FacilityIsValid(uint8_t facility);
static inline bool    FetchFromStore(char* buf, size_t maxSize, size_t* len);
static inline void    FormatCapturedTimestamp(struct SolidSyslogFormatter* f, const struct SolidSyslogTimestamp* ts);
static inline size_t  FormatMessage(char* buffer, size_t size, const struct SolidSyslogMessage* message);
static inline void    FormatMsg(struct SolidSyslogFormatter* f, const char* msg);
static inline void    FormatMsgId(struct SolidSyslogFormatter* f, const char* messageId);
static inline void    FormatNilvalue(struct SolidSyslogFormatter* f);
static inline void    FormatNonZeroUtcOffset(struct SolidSyslogFormatter* f, int16_t offsetMinutes);
static inline void    FormatPrival(struct SolidSyslogFormatter* f, uint8_t prival);
static inline void    FormatStringField(struct SolidSyslogFormatter* f, SolidSyslogStringFunction fn, size_t maxSize);
static inline void    FormatStructuredData(struct SolidSyslogFormatter* f, struct SolidSyslogStructuredData** sd, size_t sdCount);
static inline void    FormatTimestamp(struct SolidSyslogFormatter* f, SolidSyslogClockFunction clock);
static inline void    FormatUtcOffset(struct SolidSyslogFormatter* f, int16_t offsetMinutes);
static inline bool    IsServiceEnabled(void);
static inline uint8_t MakePrival(const struct SolidSyslogMessage* message);
static void           NilClock(struct SolidSyslogTimestamp* ts);
static void           NilStringFunction(struct SolidSyslogFormatter* formatter);
static inline bool    PrivalComponentsAreValid(uint8_t facility, uint8_t severity);
static void           ProcessMessages(void);
static inline bool    ReceiveFromBufferIntoStore(char* buf, size_t maxSize, size_t* len);
static inline bool    SeverityIsValid(uint8_t severity);
static inline bool    StringIsValid(const char* value);
static inline bool    TimestampIsValid(const struct SolidSyslogTimestamp* ts);

struct SolidSyslog
{
    struct SolidSyslogBuffer*          buffer;
    struct SolidSyslogSender*          sender;
    SolidSyslogClockFunction           clock;
    SolidSyslogStringFunction          getHostname;
    SolidSyslogStringFunction          getAppName;
    SolidSyslogStringFunction          getProcessId;
    struct SolidSyslogStore*           store;
    struct SolidSyslogStructuredData** sd;
    size_t                             sdCount;
};

static struct SolidSyslog instance = {
    .clock        = NilClock,
    .getHostname  = NilStringFunction,
    .getAppName   = NilStringFunction,
    .getProcessId = NilStringFunction,
};

static void NilClock(struct SolidSyslogTimestamp* ts)
{
    (void) ts;
}

static void NilStringFunction(struct SolidSyslogFormatter* formatter)
{
    (void) formatter;
}

void SolidSyslog_Create(const struct SolidSyslogConfig* config)
{
    instance.buffer = config->buffer;
    instance.sender = config->sender;
    ASSIGN_IF_NON_NULL(instance.clock, config->clock);
    ASSIGN_IF_NON_NULL(instance.getHostname, config->getHostname);
    ASSIGN_IF_NON_NULL(instance.getAppName, config->getAppName);
    ASSIGN_IF_NON_NULL(instance.getProcessId, config->getProcessId);
    instance.store   = config->store;
    instance.sd      = config->sd;
    instance.sdCount = config->sdCount;
}

void SolidSyslog_Destroy(void)
{
    instance.buffer       = NULL;
    instance.sender       = NULL;
    instance.clock        = NilClock;
    instance.getHostname  = NilStringFunction;
    instance.getAppName   = NilStringFunction;
    instance.getProcessId = NilStringFunction;
    instance.store        = NULL;
    instance.sd           = NULL;
    instance.sdCount      = 0;
}

void SolidSyslog_Service(void)
{
    if (IsServiceEnabled())
    {
        ProcessMessages();
    }
}

static inline bool IsServiceEnabled(void)
{
    return !SolidSyslogStore_IsHalted(instance.store);
}

static void ProcessMessages(void)
{
    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    size_t len = 0;

    bool haveMessage = ReceiveFromBufferIntoStore(buf, sizeof(buf), &len);
    bool fromStore   = FetchFromStore(buf, sizeof(buf), &len);

    if (fromStore || haveMessage)
    {
        if (SolidSyslogSender_Send(instance.sender, buf, len))
        {
            if (fromStore)
            {
                SolidSyslogStore_MarkSent(instance.store);
            }
        }
    }
}

static inline bool ReceiveFromBufferIntoStore(char* buf, size_t maxSize, size_t* len)
{
    bool received = SolidSyslogBuffer_Read(instance.buffer, buf, maxSize, len);

    if (received)
    {
        SolidSyslogStore_Write(instance.store, buf, *len);
    }

    return received;
}

static inline bool FetchFromStore(char* buf, size_t maxSize, size_t* len)
{
    if (SolidSyslogStore_HasUnsent(instance.store))
    {
        return SolidSyslogStore_ReadNextUnsent(instance.store, buf, maxSize, len);
    }

    return false;
}

void SolidSyslog_Log(const struct SolidSyslogMessage* message)
{
    char   buf[SOLIDSYSLOG_MAX_MESSAGE_SIZE];
    size_t len = FormatMessage(buf, sizeof(buf), message);
    SolidSyslogBuffer_Write(instance.buffer, buf, len);
}

static inline size_t FormatMessage(char* buffer, size_t size, const struct SolidSyslogMessage* message)
{
    struct SolidSyslogFormatter f;
    SolidSyslogFormatter_Create(&f, buffer, size);

    FormatPrival(&f, MakePrival(message));
    SolidSyslogFormatter_Character(&f, '1');
    SolidSyslogFormatter_Character(&f, ' ');
    FormatTimestamp(&f, instance.clock);
    SolidSyslogFormatter_Character(&f, ' ');
    FormatStringField(&f, instance.getHostname, SOLIDSYSLOG_MAX_HOSTNAME_SIZE);
    SolidSyslogFormatter_Character(&f, ' ');
    FormatStringField(&f, instance.getAppName, SOLIDSYSLOG_MAX_APP_NAME_SIZE);
    SolidSyslogFormatter_Character(&f, ' ');
    FormatStringField(&f, instance.getProcessId, SOLIDSYSLOG_MAX_PROCESS_ID_SIZE);
    SolidSyslogFormatter_Character(&f, ' ');
    FormatMsgId(&f, message->messageId);
    SolidSyslogFormatter_Character(&f, ' ');
    FormatStructuredData(&f, instance.sd, instance.sdCount);
    FormatMsg(&f, message->msg);

    return f.position;
}

static inline void FormatPrival(struct SolidSyslogFormatter* f, uint8_t prival)
{
    SolidSyslogFormatter_Character(f, '<');
    SolidSyslogFormatter_Uint32(f, prival);
    SolidSyslogFormatter_Character(f, '>');
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

static inline void FormatTimestamp(struct SolidSyslogFormatter* f, SolidSyslogClockFunction clock)
{
    struct SolidSyslogTimestamp ts = {0};

    if (CaptureTimestamp(&ts, clock))
    {
        FormatCapturedTimestamp(f, &ts);
    }
    else
    {
        FormatNilvalue(f);
    }
}

static inline bool CaptureTimestamp(struct SolidSyslogTimestamp* ts, SolidSyslogClockFunction clock)
{
    clock(ts);
    return TimestampIsValid(ts);
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

static inline void FormatCapturedTimestamp(struct SolidSyslogFormatter* f, const struct SolidSyslogTimestamp* ts)
{
    SolidSyslogFormatter_PaddedUint32(f, ts->year, 4);
    SolidSyslogFormatter_Character(f, '-');
    SolidSyslogFormatter_PaddedUint32(f, ts->month, 2);
    SolidSyslogFormatter_Character(f, '-');
    SolidSyslogFormatter_PaddedUint32(f, ts->day, 2);
    SolidSyslogFormatter_Character(f, 'T');
    SolidSyslogFormatter_PaddedUint32(f, ts->hour, 2);
    SolidSyslogFormatter_Character(f, ':');
    SolidSyslogFormatter_PaddedUint32(f, ts->minute, 2);
    SolidSyslogFormatter_Character(f, ':');
    SolidSyslogFormatter_PaddedUint32(f, ts->second, 2);
    SolidSyslogFormatter_Character(f, '.');
    SolidSyslogFormatter_PaddedUint32(f, ts->microsecond, 6);
    FormatUtcOffset(f, ts->utcOffsetMinutes);
}

static inline void FormatUtcOffset(struct SolidSyslogFormatter* f, int16_t offsetMinutes)
{
    if (offsetMinutes == 0)
    {
        SolidSyslogFormatter_Character(f, 'Z');
    }
    else
    {
        FormatNonZeroUtcOffset(f, offsetMinutes);
    }
}

static inline void FormatNonZeroUtcOffset(struct SolidSyslogFormatter* f, int16_t offsetMinutes)
{
    int16_t absoluteMinutes = AbsoluteInt16(offsetMinutes);

    SolidSyslogFormatter_Character(f, (offsetMinutes > 0) ? '+' : '-');
    SolidSyslogFormatter_PaddedUint32(f, (uint32_t) (absoluteMinutes / 60), 2);
    SolidSyslogFormatter_Character(f, ':');
    SolidSyslogFormatter_PaddedUint32(f, (uint32_t) (absoluteMinutes % 60), 2);
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

static inline void FormatStringField(struct SolidSyslogFormatter* f, SolidSyslogStringFunction fn, size_t maxSize)
{
    size_t maxChars       = maxSize - 1;
    size_t positionBefore = f->position;

    fn(f);

    size_t written = f->position - positionBefore;

    if (written > maxChars)
    {
        f->position            = positionBefore + maxChars;
        f->buffer[f->position] = '\0';
    }

    if (f->position == positionBefore)
    {
        FormatNilvalue(f);
    }
}

static inline void FormatMsgId(struct SolidSyslogFormatter* f, const char* messageId)
{
    size_t len = 0;

    if (StringIsValid(messageId))
    {
        len = SolidSyslogFormatter_BoundedString(f, messageId, SOLIDSYSLOG_MAX_MSGID_SIZE - 1);
    }

    if (len == 0)
    {
        FormatNilvalue(f);
    }
}

static inline bool StringIsValid(const char* value)
{
    return (value != NULL) && (value[0] != '\0');
}

static inline void FormatStructuredData(struct SolidSyslogFormatter* f, struct SolidSyslogStructuredData** sd, size_t sdCount)
{
    size_t positionBefore = f->position;

    for (size_t i = 0; i < sdCount; i++)
    {
        SolidSyslogStructuredData_Format(sd[i], f);
    }

    if (f->position == positionBefore)
    {
        FormatNilvalue(f);
    }
}

static inline void FormatMsg(struct SolidSyslogFormatter* f, const char* msg)
{
    if (StringIsValid(msg))
    {
        SolidSyslogFormatter_Character(f, ' ');
        SolidSyslogFormatter_BoundedString(f, msg, SolidSyslogFormatter_Remaining(f) - 1);
    }
}

static inline void FormatNilvalue(struct SolidSyslogFormatter* f)
{
    SolidSyslogFormatter_Character(f, '-');
}
