#!/usr/bin/env python3
"""Synthetic-only controlling-terminal checks; never use real credentials."""
import errno
import json
import os
from pathlib import Path
import pty
import re
import select
import signal
import subprocess
import sys
import tempfile
import termios
import time

binary = str(Path(sys.argv[1]).resolve())
key = b"synthetic_move_test_key_NOT_REAL"
count = 0


def check(path, success):
    global count
    result = subprocess.run([binary, "--check", "--config", str(path)],
                            capture_output=True, start_new_session=True)
    assert (result.returncode == 0) == success, result.stderr
    assert key not in result.stdout + result.stderr
    count += 1


def configure(path, success=True, entry=key, interrupt=False, mutate=None):
    global count
    pid, fd = pty.fork()
    if pid == 0:
        original = termios.tcgetattr(0)
        child = subprocess.Popen([binary, "--configure", "--config", str(path)])
        print(f"HELPER_PID={child.pid}", flush=True)
        status = child.wait()
        restored = termios.tcgetattr(0) == original
        print(f"HARNESS_RESULT={status},{int(restored)}", flush=True)
        os._exit(0)
    data = b""
    submitted = False
    deadline = time.monotonic() + 8
    try:
        while time.monotonic() < deadline:
            if not select.select([fd], [], [], .1)[0]:
                continue
            try:
                block = os.read(fd, 4096)
            except OSError as error:
                if error.errno == errno.EIO:
                    break
                raise
            if not block:
                break
            data += block
            if b"test): " in data and not submitted:
                submitted = True
                assert not termios.tcgetattr(fd)[3] & (termios.ECHO | termios.ECHONL)
                if mutate:
                    mutate()
                if interrupt:
                    helper_pid = int(re.search(rb"HELPER_PID=(\d+)", data)[1])
                    os.kill(helper_pid, signal.SIGTERM)
                else:
                    os.write(fd, entry + b"\n")
        else:
            raise AssertionError("Helper terminal timeout")
        result = re.search(rb"HARNESS_RESULT=(-?\d+),(\d+)", data)
        assert result and result[2] == b"1", data
        assert (int(result[1]) == 0) == success, data
        assert key not in data and (not entry or entry not in data), "Key echoed"
    finally:
        os.close(fd)
        waited, _ = os.waitpid(pid, os.WNOHANG)
        if not waited:
            os.kill(pid, signal.SIGKILL)
            os.waitpid(pid, 0)
    count += 1


with tempfile.TemporaryDirectory(prefix="move-helper-synthetic-") as root:
    directory = Path(root)
    path = directory / "private.json"
    configure(path)
    assert path.stat().st_mode & 0o777 == 0o600
    saved = json.loads(path.read_text())
    assert saved == {"api_key": key.decode(), "model": "gpt-6-luna"}
    check(path, True)
    saved.update(model="previous-model", unrelated={"nested": [1, 2]})
    path.write_text(json.dumps(saved))
    configure(path)
    assert json.loads(path.read_text())["unrelated"] == saved["unrelated"]
    original = path.read_bytes()
    configure(path, False, entry=b"invalid/key")
    assert path.read_bytes() == original
    configure(path, False, entry=b"")
    configure(path, False, entry=b"s" * 513)
    assert path.read_bytes() == original
    configure(path, False, interrupt=True)
    assert path.read_bytes() == original
    configure(path, False, mutate=lambda: path.write_text('{"changed":true}'))
    assert path.read_text() == '{"changed":true}'
    path.write_bytes(original)
    path.chmod(0o644)
    configure(path, False)
    check(path, False)
    path.chmod(0o600)
    linked = directory / "linked.json"
    linked.symlink_to(path)
    configure(linked, False)
    check(linked, False)
    linked.unlink()
    os.link(path, linked)
    configure(path, False)
    check(path, False)
    linked.unlink()
    path.write_text('{broken')
    configure(path, False)
    assert path.read_text() == '{broken'
    path.write_text(json.dumps({**saved, "demo_mode": True}))
    before = path.read_bytes()
    configure(path, False)
    assert path.read_bytes() == before
    # Valid old file, but preserving its fields plus the new key would exceed 16KiB.
    path.write_text(json.dumps({"padding": "p" * 16340}, separators=(",", ":")))
    before = path.read_bytes()
    configure(path, False)
    assert path.read_bytes() == before
    path.write_bytes(b"x" * 16385)
    configure(path, False)
    check(path, False)
    path.unlink()
    os.mkfifo(path, 0o600)
    configure(path, False)
    check(path, False)
    path.unlink()
    path.touch(mode=0o600)
    path.write_bytes(original)
    directory.chmod(0o755)
    configure(path, False)
    check(path, False)
    directory.chmod(0o700)
    alias = directory / "alias"
    alias.symlink_to(directory, target_is_directory=True)
    configure(alias / path.name, False)
    result = subprocess.run([binary, "--configure", "--config", str(path)],
                            capture_output=True, start_new_session=True)
    assert result.returncode != 0 and path.read_bytes() == original
    count += 1
print(f"PASS: {count} synthetic config/TTY cases; hidden input, exact terminal restoration, atomic preservation, unsafe-path refusal")
