Feature: Walking skeleton end-to-end
  The example program sends a hardcoded RFC 5424 message via UDP.
  syslog-ng receives it and writes the parsed fields to a log file.

  Scenario: SolidSyslog sends a valid RFC 5424 message to syslog-ng
    Given syslog-ng is running
    When the example program sends a syslog message
    Then syslog-ng receives a message with priority "134"
    And the timestamp is "2009-03-23T00:00:00+00:00"
    And the hostname is "TestHost"
    And the app name is "TestApp"
    And the process ID is "42"
    And the message ID is "54"
    And the message is "hello world"
