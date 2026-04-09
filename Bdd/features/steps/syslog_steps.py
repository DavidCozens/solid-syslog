import os
import re
import socket
import subprocess
import time
from datetime import datetime, timezone

from behave import given, when, then

RECEIVED_LOG = "Bdd/output/received.log"
EXAMPLE_BINARY = "build/debug/Example/SolidSyslogExample"
THREADED_BINARY = "build/debug/Example/SolidSyslogThreadedExample"
SYSLOG_NG_CTL = "/var/lib/syslog-ng/syslog-ng.ctl"


def line_count(path):
    """Return the number of lines in a file, or 0 if it doesn't exist."""
    if not os.path.exists(path):
        return 0
    with open(path) as f:
        return sum(1 for _ in f)


def read_new_lines(path, skip):
    """Return all lines after the first 'skip' lines."""
    with open(path) as f:
        lines = f.readlines()
    return [line.strip() for line in lines[skip:] if line.strip()]


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

    # STRUCTURED_DATA may contain spaces and special chars — capture between
    # "STRUCTURED_DATA=" and " MSG="
    sd_match = re.search(r"STRUCTURED_DATA=(.*?) MSG=", line)
    if sd_match:
        fields["STRUCTURED_DATA"] = sd_match.group(1)

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


def run_example(context, extra_args=None, binary=None, expected_messages=1):
    """Run an example binary and wait for syslog-ng to flush the message(s)."""
    binary = binary or EXAMPLE_BINARY
    assert os.path.exists(binary), (
        f"Example binary not found at {binary} — build with cmake first"
    )

    cmd = [os.path.join(".", binary)]
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

    # Wait for syslog-ng to flush the expected number of new lines
    expected_total = context.lines_before + expected_messages
    deadline = time.monotonic() + 5
    while line_count(RECEIVED_LOG) < expected_total:
        if time.monotonic() > deadline:
            actual = line_count(RECEIVED_LOG) - context.lines_before
            assert False, (
                f"syslog-ng received {actual} of {expected_messages} "
                f"messages within 5 seconds"
            )
        time.sleep(0.1)

    context.all_lines = read_new_lines(RECEIVED_LOG, context.lines_before)
    context.fields = parse_syslog_line(context.all_lines[-1])
    context.message_count = len(context.all_lines)


@when("the example program sends a syslog message")
def step_example_sends_message(context):
    run_example(context)


@when("the threaded example sends a syslog message")
def step_threaded_sends_message(context):
    run_example(context, binary=THREADED_BINARY)


@when("the threaded example sends a syslog message with transport {transport}")
def step_threaded_sends_with_transport(context, transport):
    run_example(context, ["--transport", transport], binary=THREADED_BINARY)


@when("the threaded example sends {count:d} syslog messages")
def step_threaded_sends_multiple(context, count):
    run_example(context, ["--count", str(count)], binary=THREADED_BINARY, expected_messages=count)


@when("the example program sends a message with facility {facility:d} and severity {severity:d}")
def step_example_sends_with_facility_severity(context, facility, severity):
    run_example(context, ["--facility", str(facility), "--severity", str(severity)])


@when('the example program sends a message with message ID "{msgid}"')
def step_example_sends_with_msgid(context, msgid):
    run_example(context, ["--msgid", msgid])


@when('the example program sends a message with body "{body}"')
def step_example_sends_with_body(context, body):
    run_example(context, ["--message", body])


@when('the example program sends a complete message with message ID "{msgid}" and body "{body}"')
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


@when("the example program sends {count:d} syslog messages")
def step_example_sends_multiple(context, count):
    run_example(context, ["--count", str(count)], expected_messages=count)


@then("syslog-ng receives {count:d} messages")
def step_check_message_count(context, count):
    assert context.message_count == count, (
        f"Expected {count} messages, got {context.message_count}"
    )


@then('the structured data contains sequenceId "{value}"')
def step_check_sequence_id(context, value):
    sd = context.fields.get("STRUCTURED_DATA", "")
    match = re.search(r'sequenceId="(\d+)"', sd)
    assert match, (
        f"No sequenceId found in structured data: {sd}"
    )
    assert match.group(1) == value, (
        f"Expected sequenceId {value}, got {match.group(1)}"
    )


@then('the structured data contains tzKnown "{value}"')
def step_check_tz_known(context, value):
    sd = context.fields.get("STRUCTURED_DATA", "")
    match = re.search(r'tzKnown="(\d+)"', sd)
    assert match, (
        f"No tzKnown found in structured data: {sd}"
    )
    assert match.group(1) == value, (
        f"Expected tzKnown {value}, got {match.group(1)}"
    )


@then('the structured data contains isSynced "{value}"')
def step_check_is_synced(context, value):
    sd = context.fields.get("STRUCTURED_DATA", "")
    match = re.search(r'isSynced="(\d+)"', sd)
    assert match, (
        f"No isSynced found in structured data: {sd}"
    )
    assert match.group(1) == value, (
        f"Expected isSynced {value}, got {match.group(1)}"
    )


@then('the structured data contains software "{value}"')
def step_check_software(context, value):
    sd = context.fields.get("STRUCTURED_DATA", "")
    match = re.search(r'software="([^"]*)"', sd)
    assert match, (
        f"No software found in structured data: {sd}"
    )
    assert match.group(1) == value, (
        f"Expected software {value}, got {match.group(1)}"
    )


@then('the structured data contains swVersion "{value}"')
def step_check_sw_version(context, value):
    sd = context.fields.get("STRUCTURED_DATA", "")
    match = re.search(r'swVersion="([^"]*)"', sd)
    assert match, (
        f"No swVersion found in structured data: {sd}"
    )
    assert match.group(1) == value, (
        f"Expected swVersion {value}, got {match.group(1)}"
    )


@then("syslog-ng receives {count:d} messages with sequential sequenceId values")
def step_check_sequential_ids(context, count):
    assert context.message_count == count, (
        f"Expected {count} messages, got {context.message_count}"
    )
    for i, line in enumerate(context.all_lines, start=1):
        fields = parse_syslog_line(line)
        sd = fields.get("STRUCTURED_DATA", "")
        match = re.search(r'sequenceId="(\d+)"', sd)
        assert match, (
            f"Message {i}: no sequenceId in structured data: {sd}"
        )
        assert match.group(1) == str(i), (
            f"Message {i}: expected sequenceId {i}, got {match.group(1)}"
        )
