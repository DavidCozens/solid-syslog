import subprocess
from behave import when, then


@when("I run the example program")
def step_run_example(context):
    result = subprocess.run(
        [context.example_binary],
        capture_output=True,
    )
    context.exit_code = result.returncode


@then("the exit code should be {expected_code:d}")
def step_check_exit_code(context, expected_code):
    assert context.exit_code == expected_code, (
        f"Expected exit code {expected_code}, got {context.exit_code}"
    )
