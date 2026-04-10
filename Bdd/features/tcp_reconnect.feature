Feature: TCP reconnection after server outage
  The TCP sender detects connection failure and reconnects when the
  server recovers. Messages sent during the outage are lost (store
  and forward is a separate feature).

  @wip
  Scenario: Message delivered after server recovery
    Given syslog-ng is running
    And the threaded example is running with transport tcp
    When the client sends a message
    Then syslog-ng receives 1 messages
    When the syslog server stops accepting TCP connections
    And the client sends a message
    And the syslog server resumes accepting TCP connections
    And the client sends a message
    Then syslog-ng receives 2 messages
