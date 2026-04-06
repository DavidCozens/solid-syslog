@wip
Feature: Structured data — origin
  The library includes origin metadata identifying the software component.

  Scenario: Origin software appears in structured data
    Given syslog-ng is running
    When the example program sends a syslog message
    Then the structured data contains software "SolidSyslogExample"

  Scenario: Origin version appears in structured data
    Given syslog-ng is running
    When the example program sends a syslog message
    Then the structured data contains swVersion "0.7.0"

  Scenario: All standard structured data present
    Given syslog-ng is running
    When the example program sends a syslog message
    Then the structured data contains sequenceId "1"
    And the structured data contains tzKnown "1"
    And the structured data contains software "SolidSyslogExample"
