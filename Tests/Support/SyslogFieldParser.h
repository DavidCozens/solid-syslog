#ifndef SYSLOGFIELDPARSER_H
#define SYSLOGFIELDPARSER_H

#include <string>

// The UTF-8 BOM (U+FEFF) that prefixes an RFC 5424 MSG-UTF8 body.
extern const char UTF8_BOM[];

// RFC 5424 SYSLOG-MSG field indices, counting space-separated header fields
// from 0; SDATA (6) and the MSG body need bracket/BOM-aware skipping rather
// than plain space splitting.
enum
{
    SYSLOG_FIELD_HEADER = 0,
    SYSLOG_FIELD_TIMESTAMP = 1,
    SYSLOG_FIELD_HOSTNAME = 2,
    SYSLOG_FIELD_APP_NAME = 3,
    SYSLOG_FIELD_PROCID = 4,
    SYSLOG_FIELD_MSGID = 5,
    SYSLOG_FIELD_SDATA = 6
};

// Returns field n of a NUL-terminated RFC 5424 wire frame, or "" if absent.
std::string SyslogField(const char* buffer, int n);

// Returns the MSG body with any leading UTF-8 BOM stripped, or "" if absent.
std::string SyslogMsg(const char* buffer);

// True if the MSG body begins with the UTF-8 BOM.
bool SyslogMsgHasBom(const char* buffer);

#endif /* SYSLOGFIELDPARSER_H */
