@tcp @buffered
Feature: TCP message delivery
  The threaded example sends messages via TCP transport with
  RFC 6587 octet-counting framing to syslog-ng.

  Scenario: Message delivered over TCP
    Given syslog-ng is running
    When the threaded example sends a syslog message with transport tcp
    Then syslog-ng receives a message with priority "134"
