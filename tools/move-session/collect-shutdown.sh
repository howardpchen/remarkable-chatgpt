#!/bin/sh
# READ ONLY. Run on the Move before and after a separately authorized single
# stop/start. No service changes, core dumps, notebook paths, credentials or
# unrestricted journal export. Redirect output to private evidence storage.
set -eu
printf '=== UTC ===\n'
date -u
printf '=== Xochitl runtime ===\n'
systemctl show xochitl -p MainPID -p ActiveState -p SubState -p Result -p NRestarts -p ExecMainCode -p ExecMainStatus
printf '=== Boot and core routing metadata (not core data) ===\n'
cat /proc/sys/kernel/random/boot_id /proc/sys/kernel/core_pattern
printf '=== Filtered current and previous boot lifecycle ===\n'
for boot in 0 -1; do
    printf 'boot=%s\n' "$boot"
    journalctl -b "$boot" -u xochitl --no-pager -n 2000 -o short-precise 2>/dev/null |
        awk '/ink-move lifecycle|Main process exited|Failed with result|Stopping |Stopped |Starting |Started |Successfully captured stacktrace/'
    journalctl -b "$boot" -k --no-pager -n 2000 -o short-precise 2>/dev/null |
        awk '/xochitl.*(segfault|SIGSEGV|fault)|segfault.*xochitl/'
done
