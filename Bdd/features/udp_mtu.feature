@udp
Feature: UDP datagram path-MTU clipping
  When a UDP message exceeds the path MTU the sender clips it to the
  path-MTU's safe payload, walking back over any partial UTF-8
  codepoint at the trim point so the receiver always sees valid UTF-8.

  The oversize scenario is POSIX-only (tagged windows_wip): the
  Windows BDD runner uses the OTel Collector on 127.0.0.1, and
  loopback's ~65535-byte MTU never triggers WSAEMSGSIZE for the
  message sizes we can produce inside SOLIDSYSLOG_MAX_MESSAGE_SIZE.
  The full-delivery scenario runs on both runners now that S12.13
  (#219) ships the RFC 5424 §6.4 BOM, which is what the OTel syslog
  receiver needs to parse MSG as UTF-8 instead of Latin-1.

  Scenario: Full delivery of a UTF-8 message within the path MTU
    Given syslog-ng is running
    When the example program sends a UTF-8 message that fits the path MTU
    Then the received message is byte-identical to the sent message

  @windows_wip
  Scenario: Oversize UTF-8 message is clipped at a codepoint boundary
    Given syslog-ng is running
    When the example program sends an oversize UTF-8 message
    Then the received message is shorter than the sent message
    And the received message is a clean prefix of the sent message
