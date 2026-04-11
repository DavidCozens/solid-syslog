Feature: Store capacity limit and discard policy
  The file store uses rotating files with a configurable capacity.
  When the store is full, the discard policy determines whether
  the oldest or newest messages are dropped.

  Scenario: Discard-oldest drops oldest messages when store overflows
    Given syslog-ng is running
    And the file store is enabled with max-files 2 and max-file-size 1100 and discard-policy oldest
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then syslog-ng receives 1 message
    When the syslog server stops accepting TCP connections
    And the client sends 6 messages
    And the syslog server resumes accepting TCP connections
    Then syslog-ng receives 5 messages
    And the last 4 messages have contiguous sequenceIds starting from 4

  Scenario: Discard-newest preserves oldest messages when store overflows
    Given syslog-ng is running
    And the file store is enabled with max-files 2 and max-file-size 1100 and discard-policy newest
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then syslog-ng receives 1 message
    When the syslog server stops accepting TCP connections
    And the client sends 6 messages
    And the syslog server resumes accepting TCP connections
    Then syslog-ng receives 5 messages
    And the last 4 messages have contiguous sequenceIds starting from 2
