@buffered
Feature: Switch transport at runtime
  The threaded example always wraps its UDP and TCP senders in a
  SwitchingSender. The --transport CLI flag sets the initial selector
  value, and the interactive `switch` command updates it at runtime.

  Scenario: Switch from UDP to TCP mid-run delivers via both
    Given syslog-ng is running
    And the switching example is running with default transport udp
    When the client sends a message
    Then syslog-ng receives 1 message over udp
    When the client switches to transport tcp
    And the client sends a message
    Then syslog-ng receives 1 message over tcp
