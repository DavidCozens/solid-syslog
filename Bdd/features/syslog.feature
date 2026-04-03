Feature: Walking skeleton end-to-end
  The example program sends an RFC 5424 message via UDP.
  syslog-ng receives it and writes the parsed fields to a log file.

  Scenario: SolidSyslog sends a valid RFC 5424 message to syslog-ng
    Given syslog-ng is running
    When the example program sends a syslog message
    Then syslog-ng receives a message with priority "134"
    And syslog-ng receives a message with a timestamp within 5 seconds of now
    And syslog-ng receives a message with the system hostname
    And the app name is "SolidSyslogExample"
    And syslog-ng receives a message with the process ID of the example program
    And the message ID is "54"
    And the message is "hello world"
