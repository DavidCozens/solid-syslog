import logging
import os
import shutil
import socket
import time

logger = logging.getLogger("behave.environment")

SYSLOG_NG_CONF = "Bdd/syslog-ng/syslog-ng.conf"
SYSLOG_NG_FULL_CONF = "Bdd/syslog-ng/syslog-ng-full.conf"
SYSLOG_NG_CTL = "/var/lib/syslog-ng/syslog-ng.ctl"


def before_all(context):
    context.example_binary = os.environ.get(
        "EXAMPLE_BINARY", "build/debug/Example/ExampleProgram"
    )


def after_scenario(context, scenario):
    # Clean up any long-lived interactive process
    if hasattr(context, "interactive_process"):
        process = context.interactive_process
        if process.poll() is None:
            try:
                process.stdin.write("quit\n")
                process.stdin.flush()
            except (BrokenPipeError, OSError):
                pass
            try:
                process.wait(timeout=5)
            except Exception:
                process.kill()
                process.wait()
        del context.interactive_process

    # Restore syslog-ng config if it was changed during the scenario
    if hasattr(context, "syslog_ng_config_changed") and context.syslog_ng_config_changed:
        shutil.copy(SYSLOG_NG_FULL_CONF, SYSLOG_NG_CONF)
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
                sock.connect(SYSLOG_NG_CTL)
                sock.sendall(b"RELOAD\n")
                sock.recv(1024)
            time.sleep(0.5)
        except Exception as exc:
            logger.warning("Failed to restore syslog-ng config in teardown: %s", exc)
        context.syslog_ng_config_changed = False
