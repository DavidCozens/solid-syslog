#ifndef SOLIDSYSLOGSERVICESTATUS_H
#define SOLIDSYSLOGSERVICESTATUS_H

/** Advisory servicing hint returned by SolidSyslog_Service, which documents
 *  each state and the host action it suggests. */
enum SolidSyslogServiceStatus
{
    SOLIDSYSLOG_SERVICE_IDLE,
    SOLIDSYSLOG_SERVICE_READY,
    SOLIDSYSLOG_SERVICE_BLOCKED,
    SOLIDSYSLOG_SERVICE_HALTED
};

#endif /* SOLIDSYSLOGSERVICESTATUS_H */
