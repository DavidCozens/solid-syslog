Feature: Example program

  The example program calls the library and returns the result as its exit code.

  Scenario: Running the example returns the answer
    When I run the example program
    Then the exit code should be 42
