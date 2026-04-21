@tls @buffered
Feature: TLS message delivery
  The threaded example sends messages via TLS transport (RFC 5425) —
  RFC 6587 octet-counting framing over TLS — to syslog-ng.

  Scenario: Message delivered over TLS
    Given syslog-ng is running
    When the threaded example sends a syslog message with transport tls
    Then syslog-ng receives 1 message over tls
    And syslog-ng receives a message with priority "134"
