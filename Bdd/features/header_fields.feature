@udp @windows_wip
Feature: Message header fields
  The library includes hostname, app-name, and process ID in the
  RFC 5424 message header.

  Scenario: Hostname matches the system hostname
    Given syslog-ng is running
    When the example program sends a syslog message
    Then syslog-ng receives a message with the system hostname

  Scenario: App name matches the example program
    Given syslog-ng is running
    When the example program sends a syslog message
    Then the app name is "SolidSyslogExample"

  Scenario: Process ID matches the example program PID
    Given syslog-ng is running
    When the example program sends a syslog message
    Then syslog-ng receives a message with the process ID of the example program
