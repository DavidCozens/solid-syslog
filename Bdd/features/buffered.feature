Feature: Buffered message delivery
  The threaded example sends messages via a PosixMessageQueueBuffer.
  A service thread drains the buffer and sends via UDP to syslog-ng.

  Scenario: Single buffered message arrives at syslog-ng
    Given syslog-ng is running
    When the threaded example sends a syslog message
    Then syslog-ng receives a message with priority "134"
    And syslog-ng receives a message with a timestamp within 5 seconds of now

  Scenario: Multiple buffered messages arrive at syslog-ng
    Given syslog-ng is running
    When the threaded example sends 3 syslog messages
    Then syslog-ng receives 3 messages
