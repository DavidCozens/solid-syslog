@udp @windows_wip
Feature: Structured data — sequence ID
  The library includes an auto-incrementing sequence ID in structured data.

  Scenario: First message has sequence ID 1
    Given syslog-ng is running
    When the example program sends a syslog message
    Then the structured data contains sequenceId "1"

  Scenario: Sequence ID increments with each message
    Given syslog-ng is running
    When the example program sends 3 syslog messages
    Then syslog-ng receives 3 messages with sequential sequenceId values
