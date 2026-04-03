import os
import re
import socket
import subprocess
import time
from datetime import datetime, timezone

from behave import given, when, then

RECEIVED_LOG = "Bdd/output/received.log"
EXAMPLE_BINARY = "build/debug/Example/SolidSyslogExample"
SYSLOG_NG_CTL = "/var/lib/syslog-ng/syslog-ng.ctl"


def line_count(path):
    """Return the number of lines in a file, or 0 if it doesn't exist."""
    if not os.path.exists(path):
        return 0
    with open(path) as f:
        return sum(1 for _ in f)


def read_last_line(path):
    """Return the last non-empty line from a file."""
    with open(path) as f:
        lines = f.readlines()
    for line in reversed(lines):
        if line.strip():
            return line.strip()
    return ""


def parse_syslog_line(line):
    """Parse a syslog-ng key=value template line into a dict."""
    fields = {}
    for match in re.finditer(r"(\w+)=(\S+)", line):
        fields[match.group(1)] = match.group(2)

    # MSG may contain spaces — capture everything after "MSG="
    msg_match = re.search(r"MSG=(.*)", line)
    if msg_match:
        fields["MSG"] = msg_match.group(1)

    return fields


@given("syslog-ng is running")
def step_syslog_ng_is_running(context):
    assert os.path.exists(SYSLOG_NG_CTL), (
        f"syslog-ng control socket not found at {SYSLOG_NG_CTL}"
    )

    # Record current line count so we can detect the new message
    context.lines_before = line_count(RECEIVED_LOG)


def run_example(context, extra_args=None):
    """Run the example binary and wait for syslog-ng to flush the message."""
    assert os.path.exists(EXAMPLE_BINARY), (
        f"Example binary not found at {EXAMPLE_BINARY} — build with cmake first"
    )

    cmd = [os.path.join(".", EXAMPLE_BINARY)]
    if extra_args:
        cmd.extend(extra_args)

    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    context.example_pid = process.pid
    stdout, stderr = process.communicate(timeout=10)
    assert process.returncode == 0, (
        f"Example binary failed: {stderr}"
    )

    # Wait for syslog-ng to flush a new line to disk
    deadline = time.monotonic() + 5
    while line_count(RECEIVED_LOG) <= context.lines_before:
        if time.monotonic() > deadline:
            assert False, "syslog-ng did not write a new line within 5 seconds"
        time.sleep(0.1)

    context.fields = parse_syslog_line(read_last_line(RECEIVED_LOG))


@when("the example program sends a syslog message")
def step_example_sends_message(context):
    run_example(context)


@when("the example program sends a message with facility {facility:d} and severity {severity:d}")
def step_example_sends_with_facility_severity(context, facility, severity):
    run_example(context, ["--facility", str(facility), "--severity", str(severity)])


@when('the example program sends a message with message ID "{msgid}"')
def step_example_sends_with_msgid(context, msgid):
    run_example(context, ["--msgid", msgid])


@when('the example program sends a message with body "{body}"')
def step_example_sends_with_body(context, body):
    run_example(context, ["--message", body])


@when('the example program sends a message with message ID "{msgid}" and body "{body}"')
def step_example_sends_with_msgid_and_body(context, msgid, body):
    run_example(context, ["--msgid", msgid, "--message", body])


@then('syslog-ng receives a message with priority "{priority}"')
def step_check_priority(context, priority):
    assert context.fields["PRIORITY"] == priority, (
        f"Expected priority {priority}, got {context.fields.get('PRIORITY')}"
    )


@then('the timestamp is "{timestamp}"')
def step_check_timestamp(context, timestamp):
    assert context.fields["TIMESTAMP"] == timestamp, (
        f"Expected timestamp {timestamp}, got {context.fields.get('TIMESTAMP')}"
    )


@then("syslog-ng receives a message with a timestamp within {seconds:d} seconds of now")
def step_check_timestamp_within(context, seconds):
    raw = context.fields["TIMESTAMP"]
    received = datetime.fromisoformat(raw).astimezone(timezone.utc)
    now = datetime.now(timezone.utc)
    delta = abs((now - received).total_seconds())
    assert delta <= seconds, (
        f"Timestamp {raw} is {delta:.1f}s from now, expected within {seconds}s"
    )


@then("syslog-ng receives a message with the system hostname")
def step_check_system_hostname(context):
    expected = socket.gethostname()
    assert context.fields["HOSTNAME"] == expected, (
        f"Expected hostname {expected}, got {context.fields.get('HOSTNAME')}"
    )


@then("syslog-ng receives a message with the process ID of the example program")
def step_check_example_pid(context):
    expected = str(context.example_pid)
    assert context.fields["PROCID"] == expected, (
        f"Expected PID {expected}, got {context.fields.get('PROCID')}"
    )


@then('the hostname is "{hostname}"')
def step_check_hostname(context, hostname):
    assert context.fields["HOSTNAME"] == hostname, (
        f"Expected hostname {hostname}, got {context.fields.get('HOSTNAME')}"
    )


@then('the app name is "{app_name}"')
def step_check_app_name(context, app_name):
    assert context.fields["APP_NAME"] == app_name, (
        f"Expected app name {app_name}, got {context.fields.get('APP_NAME')}"
    )


@then('the process ID is "{procid}"')
def step_check_procid(context, procid):
    assert context.fields["PROCID"] == procid, (
        f"Expected process ID {procid}, got {context.fields.get('PROCID')}"
    )


@then('the message ID is "{msgid}"')
def step_check_msgid(context, msgid):
    assert context.fields["MSGID"] == msgid, (
        f"Expected message ID {msgid}, got {context.fields.get('MSGID')}"
    )


@then('the message is "{msg}"')
def step_check_msg(context, msg):
    assert context.fields["MSG"] == msg, (
        f"Expected message {msg}, got {context.fields.get('MSG')}"
    )
