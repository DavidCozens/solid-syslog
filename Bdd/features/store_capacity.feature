@tcp @buffered
Feature: Store capacity limit and discard policy
  The block store uses rotating files with a configurable capacity.
  When the store is full, the discard policy determines whether
  the oldest or newest messages are dropped.

  # Outage uses 20 messages (seqIds 2-21) to guarantee BlockStore overflow on
  # both runners. The Linux compose runner has a clean outage; the Windows
  # OTel runner can briefly leak 1-3 records through otelcol's shutdown flush
  # before taskkill completes, so 10 messages was tight (leak + capacity-8
  # store could just barely fit without overflow). 20 always overflows by
  # at least 9 records, regardless of leak.
  Scenario: Discard-oldest drops oldest messages when store overflows
    Given the syslog oracle is running
    And the block store is enabled with max-blocks 2 and max-block-size 520 and discard-policy oldest
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then the syslog oracle receives 1 message
    When the syslog oracle stops accepting TCP connections
    And the client sends 20 messages
    And the syslog oracle resumes accepting TCP connections
    Then the syslog oracle finishes draining
    And the syslog oracle received sequenceId 1
    And the syslog oracle received sequenceId 21
    And the syslog oracle did not receive sequenceId 13

  Scenario: Discard-newest preserves oldest messages when store overflows
    Given the syslog oracle is running
    And the block store is enabled with max-blocks 2 and max-block-size 520 and discard-policy newest
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then the syslog oracle receives 1 message
    When the syslog oracle stops accepting TCP connections
    And the client sends 20 messages
    And the syslog oracle resumes accepting TCP connections
    Then the syslog oracle finishes draining
    And the syslog oracle received sequenceId 1
    And the syslog oracle did not receive sequenceId 21

  Scenario: Halt stops the application when store overflows
    Given the syslog oracle is running
    And the block store is enabled with max-blocks 2 and max-block-size 520 and discard-policy halt
    And the halt callback exits the process
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then the syslog oracle receives 1 message
    When the syslog oracle stops accepting TCP connections
    And the client sends 20 messages
    And the client attempts to send it exits with code 2

  Scenario: Halt prevents further service after store overflows
    Given the syslog oracle is running
    And the block store is enabled with max-blocks 2 and max-block-size 520 and discard-policy halt
    And the threaded example is running with transport tcp and no structured data
    When the client sends a message
    Then the syslog oracle receives 1 message
    When the syslog oracle stops accepting TCP connections
    And the client sends 20 messages
    And the syslog oracle resumes accepting TCP connections
    Then the syslog oracle receives no more messages
