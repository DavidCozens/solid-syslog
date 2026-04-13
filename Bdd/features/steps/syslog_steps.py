import glob
import os
import re
import shutil
import signal
import socket
import subprocess
import time
from datetime import datetime, timezone

from behave import given, when, then
from environment import STORE_FILE_PATH, STORE_PATH_PREFIX

RECEIVED_LOG = "Bdd/output/received.log"
EXAMPLE_BINARY = "build/debug/Example/SolidSyslogExample"
THREADED_BINARY = "build/debug/Example/SolidSyslogThreadedExample"
SYSLOG_NG_CTL = "/var/lib/syslog-ng/syslog-ng.ctl"
SYSLOG_NG_CONF = "Bdd/syslog-ng/syslog-ng.conf"
SYSLOG_NG_FULL_CONF = "Bdd/syslog-ng/syslog-ng-full.conf"
SYSLOG_NG_UDP_ONLY_CONF = "Bdd/syslog-ng/syslog-ng-udp-only.conf"


def clean_store_files():
    """Remove all rotating store files matching the path prefix."""
    for path in glob.glob(STORE_PATH_PREFIX + "*.log"):
        os.remove(path)


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


def wait_for_prompt(process, timeout=30):
    """Read stdout until we see 'SolidSyslog> ', confirming the command completed."""
    import select

    fd = process.stdout.fileno()
    output = b""
    deadline = time.monotonic() + timeout
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(
                f"Timed out waiting for prompt after {timeout}s. "
                f"Output so far: {output.decode(errors='replace')}"
            )
        ready, _, _ = select.select([fd], [], [], min(remaining, 0.5))
        if not ready:
            continue
        data = os.read(fd, 1)
        if not data:
            break
        output += data
        if output.endswith(b"SolidSyslog> "):
            return output.decode()
    return output.decode()


def send_command(process, command):
    """Send a command to the interactive process and wait for the prompt."""
    process.stdin.write(command + "\n")
    process.stdin.flush()
    return wait_for_prompt(process)


def wait_for_messages(context, expected_messages):
    """Wait for syslog-ng to flush the expected number of new lines."""
    expected_total = context.lines_before + expected_messages
    deadline = time.monotonic() + 5
    while line_count(RECEIVED_LOG) < expected_total:
        if time.monotonic() > deadline:
            actual = line_count(RECEIVED_LOG) - context.lines_before
            raise AssertionError(
                f"syslog-ng received {actual} of {expected_messages} "
                f"messages within 5 seconds"
            )
        time.sleep(0.1)

    context.all_lines = read_new_lines(RECEIVED_LOG, context.lines_before)
    context.fields = parse_syslog_line(context.all_lines[-1])
    context.message_count = len(context.all_lines)


def run_example(context, extra_args=None, binary=None, expected_messages=1):
    """Run an example binary, send messages via stdin, wait for delivery."""
    binary = binary or EXAMPLE_BINARY
    assert os.path.exists(binary), (
        f"Example binary not found at {binary} — build with cmake first"
    )

    cmd = [os.path.join(".", binary)]
    if extra_args:
        cmd.extend(extra_args)

    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    context.example_pid = process.pid

    # Wait for initial prompt
    wait_for_prompt(process)

    # Send messages and wait for confirmation
    send_command(process, f"send {expected_messages}")

    # Wait for syslog-ng to receive the messages before quitting
    wait_for_messages(context, expected_messages)

    # Clean shutdown — write quit, don't wait for prompt (process exits)
    process.stdin.write("quit\n")
    process.stdin.flush()
    process.wait(timeout=10)
    assert process.returncode == 0, (
        f"Example binary failed with exit code {process.returncode}"
    )


def syslog_ng_reload():
    """Send RELOAD to syslog-ng via its Unix control socket."""
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
        sock.connect(SYSLOG_NG_CTL)
        sock.sendall(b"RELOAD\n")
        response = sock.recv(1024)
    assert b"OK" in response, f"syslog-ng reload failed: {response}"
    # Allow time for syslog-ng to complete the reload
    time.sleep(0.5)


def syslog_ng_swap_config(config_path):
    """Replace the active syslog-ng config and reload."""
    shutil.copy(config_path, SYSLOG_NG_CONF)
    syslog_ng_reload()


@given("syslog-ng is running")
def step_syslog_ng_is_running(context):
    assert os.path.exists(SYSLOG_NG_CTL), (
        f"syslog-ng control socket not found at {SYSLOG_NG_CTL}"
    )

    # Record current line count so we can detect the new message
    context.lines_before = line_count(RECEIVED_LOG)


def build_threaded_command(context, transport, no_sd=False):
    """Build the command line for the threaded example with all options."""
    binary = THREADED_BINARY
    assert os.path.exists(binary), (
        f"Threaded binary not found at {binary} — build with cmake first"
    )

    cmd = [os.path.join(".", binary), "--transport", transport]
    if getattr(context, "store_type", None):
        cmd.extend(["--store", context.store_type])
    if getattr(context, "store_max_files", None):
        cmd.extend(["--max-files", str(context.store_max_files)])
    if getattr(context, "store_max_file_size", None):
        cmd.extend(["--max-file-size", str(context.store_max_file_size)])
    if getattr(context, "store_discard_policy", None):
        cmd.extend(["--discard-policy", context.store_discard_policy])
    if no_sd:
        cmd.append("--no-sd")
    if getattr(context, "halt_exit", False):
        cmd.append("--halt-exit")
    return cmd


def start_threaded_example(context, cmd):
    """Start the threaded example process and wait for the initial prompt."""
    context.interactive_process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    context.example_pid = context.interactive_process.pid
    wait_for_prompt(context.interactive_process)


@given("the threaded example is running with transport {transport:w}")
def step_threaded_running_with_transport(context, transport):
    cmd = build_threaded_command(context, transport)
    start_threaded_example(context, cmd)


@given("the threaded example is running with transport {transport:w} and no structured data")
def step_threaded_running_with_transport_no_sd(context, transport):
    cmd = build_threaded_command(context, transport, no_sd=True)
    start_threaded_example(context, cmd)


@given("the file store is enabled")
def step_file_store_enabled(context):
    context.store_type = "file"
    if os.path.exists(STORE_FILE_PATH):
        os.remove(STORE_FILE_PATH)


@given("the file store is enabled with max-files {max_files:d} and max-file-size {max_file_size:d} and discard-policy {policy}")
def step_file_store_enabled_with_config(context, max_files, max_file_size, policy):
    context.store_type = "file"
    context.store_max_files = max_files
    context.store_max_file_size = max_file_size
    context.store_discard_policy = policy
    clean_store_files()


@given("the halt callback exits the process")
def step_halt_callback_exits(context):
    context.halt_exit = True


@when("the client sends a message")
def step_client_sends_message(context):
    send_command(context.interactive_process, "send")
    # Allow time for the service thread to drain the buffer and send
    time.sleep(0.2)


def wait_for_tcp_port_closed(host="syslog-ng", port=5514, timeout=5):
    """Poll until the TCP port refuses connections."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(0.5)
            s.connect((host, port))
            s.close()
            time.sleep(0.1)
        except (ConnectionRefusedError, OSError):
            return
    raise AssertionError(f"TCP port {port} still open after {timeout}s")


def wait_for_tcp_port_open(host="syslog-ng", port=5514, timeout=5):
    """Poll until the TCP port accepts connections."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(0.5)
            s.connect((host, port))
            s.close()
            return
        except (ConnectionRefusedError, OSError):
            time.sleep(0.1)
    raise AssertionError(f"TCP port {port} not open after {timeout}s")


def wait_for_connection_teardown(probe_socket, timeout=5):
    """Wait until an established TCP connection is broken by the server.

    Sends data on the probe socket until a broken pipe or reset indicates
    that syslog-ng has closed the connection after a config reload.
    """
    msg = b"<134>1 2026-01-01T00:00:00Z probe probe - - - probe"
    frame = f"{len(msg)} ".encode() + msg
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            probe_socket.sendall(frame)
            time.sleep(0.1)
        except (BrokenPipeError, ConnectionResetError, OSError):
            probe_socket.close()
            return
    probe_socket.close()
    raise AssertionError(f"Probe connection still alive after {timeout}s")


@when("the syslog server stops accepting TCP connections")
def step_syslog_server_stops_tcp(context):
    # Open a probe connection before the reload so we can detect teardown
    probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    probe.settimeout(1)
    probe.connect(("syslog-ng", 5514))

    syslog_ng_swap_config(SYSLOG_NG_UDP_ONLY_CONF)
    wait_for_tcp_port_closed()
    wait_for_connection_teardown(probe)
    # Allow time for the sender's existing connection to receive RST
    time.sleep(0.5)
    context.syslog_ng_config_changed = True


@when("the syslog server resumes accepting TCP connections")
def step_syslog_server_resumes_tcp(context):
    syslog_ng_swap_config(SYSLOG_NG_FULL_CONF)
    wait_for_tcp_port_open()


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
    run_example(context, binary=THREADED_BINARY, expected_messages=count)


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


@when("the example program sends {count:d} syslog messages")
def step_example_sends_multiple(context, count):
    run_example(context, expected_messages=count)


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


@then("syslog-ng receives {count:d} message")
@then("syslog-ng receives {count:d} messages")
def step_check_message_count(context, count):
    # For interactive processes, refresh the line count
    if hasattr(context, "interactive_process"):
        wait_for_messages(context, count)
    assert context.message_count == count, (
        f"Expected {count} messages, got {context.message_count}"
    )


@then("syslog-ng receives no more messages")
def step_check_no_more_messages(context):
    before = line_count(RECEIVED_LOG)
    time.sleep(5)
    after = line_count(RECEIVED_LOG)
    assert after == before, (
        f"Expected no more messages, but received {after - before} additional"
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


@when("the client sends {count:d} messages")
def step_client_sends_n_messages(context, count):
    send_command(context.interactive_process, f"send {count}")
    # Allow time for the service thread to drain the buffer
    time.sleep(0.5)


@when("the client attempts to send it exits with code {code:d}")
def step_client_attempts_send_exits(context, code):
    process = context.interactive_process
    # Allow the service thread to drain the buffer and fill the store
    time.sleep(3)
    try:
        process.stdin.write("send\n")
        process.stdin.flush()
    except (BrokenPipeError, OSError):
        pass
    process.wait(timeout=10)
    assert process.returncode == code, (
        f"Expected exit code {code}, got {process.returncode}"
    )
    del context.interactive_process


@when("the client is killed")
def step_client_is_killed(context):
    context.interactive_process.send_signal(signal.SIGKILL)
    context.interactive_process.wait(timeout=5)
    del context.interactive_process


@then("the messages have contiguous sequenceIds")
def step_check_contiguous_sequence_ids(context):
    ids = []
    for line in context.all_lines:
        fields = parse_syslog_line(line)
        sd = fields.get("STRUCTURED_DATA", "")
        match = re.search(r'sequenceId="(\d+)"', sd)
        assert match, (
            f"No sequenceId in structured data: {sd}"
        )
        ids.append(int(match.group(1)))
    # Check that IDs form a contiguous ascending sequence
    for i in range(1, len(ids)):
        assert ids[i] == ids[i - 1] + 1, (
            f"Non-contiguous sequenceIds: {ids[i - 1]} followed by {ids[i]}"
        )


@then("the last {count:d} messages have contiguous sequenceIds starting from {start:d}")
def step_check_last_n_contiguous_ids(context, count, start):
    assert len(context.all_lines) >= count, (
        f"Expected at least {count} messages, got {len(context.all_lines)}"
    )
    last_n = context.all_lines[-count:]
    for i, line in enumerate(last_n):
        fields = parse_syslog_line(line)
        sd = fields.get("STRUCTURED_DATA", "")
        match = re.search(r'sequenceId="(\d+)"', sd)
        assert match, (
            f"Message {i + 1} of last {count}: no sequenceId in structured data: {sd}"
        )
        expected = start + i
        actual = int(match.group(1))
        assert actual == expected, (
            f"Message {i + 1} of last {count}: expected sequenceId {expected}, "
            f"got {actual}"
        )


@then("the replayed messages have sequenceIds {id_list}")
def step_check_replayed_sequence_ids(context, id_list):
    expected = [int(x.strip()) for x in id_list.split(",")]
    assert len(context.all_lines) >= len(expected), (
        f"Expected at least {len(expected)} messages, "
        f"got {len(context.all_lines)}"
    )
    # Replayed messages are the most recent batch excluding the first message
    # from the previous session (already verified)
    replayed = context.all_lines[-len(expected):]
    for i, line in enumerate(replayed):
        fields = parse_syslog_line(line)
        sd = fields.get("STRUCTURED_DATA", "")
        match = re.search(r'sequenceId="(\d+)"', sd)
        assert match, (
            f"Replayed message {i + 1}: no sequenceId in structured data: {sd}"
        )
        actual = int(match.group(1))
        assert actual == expected[i], (
            f"Replayed message {i + 1}: expected sequenceId {expected[i]}, "
            f"got {actual}"
        )


@then("the last message has sequenceId {value:d}")
def step_check_last_sequence_id(context, value):
    assert context.all_lines, "No messages received to check last sequenceId"
    fields = parse_syslog_line(context.all_lines[-1])
    sd = fields.get("STRUCTURED_DATA", "")
    match = re.search(r'sequenceId="(\d+)"', sd)
    assert match, (
        f"No sequenceId in last message structured data: {sd}"
    )
    actual = int(match.group(1))
    assert actual == value, (
        f"Last message: expected sequenceId {value}, got {actual}"
    )
