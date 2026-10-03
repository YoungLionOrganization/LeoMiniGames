#!/usr/bin/env python3
"""Reject application crashes without attributing other Android processes to it."""
import argparse
from pathlib import Path
import re

APP_ID = 'xyz.younglion.leominigames'
FATAL = re.compile(r'FATAL EXCEPTION|Fatal signal|UnsatisfiedLinkError|dlopen failed|CANNOT LINK EXECUTABLE')
THREADTIME = re.compile(r'^\d\d-\d\d\s+\S+\s+(\d+)\s+\d+\s+[VDIWEF]\s')


def application_crashes(text, pids, app_id=APP_ID):
    pids = set(map(str, pids))
    # Include previous app processes too, so a crash followed by restart fails.
    for line in text.splitlines():
        match = re.search(r'Start proc (\d+):'+re.escape(app_id)+r'(?:/|\s)', line)
        if not match:
            match = re.search(r'Process: '+re.escape(app_id)+r', PID: (\d+)', line)
        if match:
            pids.add(match.group(1))
    failures = []
    for line in text.splitlines():
        pid = THREADTIME.match(line)
        if FATAL.search(line) and ((pid and pid.group(1) in pids) or app_id in line):
            failures.append(line)
        if re.search(r'>>> '+re.escape(app_id)+r' <<<', line):
            failures.append(line)
    return failures


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('logcat', type=Path)
    parser.add_argument('pid_file', type=Path)
    args = parser.parse_args()
    pids = args.pid_file.read_text().split()
    if not pids or any(not p.isdecimal() for p in pids):
        raise SystemExit('FAIL: application is not alive after launch')
    failures = application_crashes(args.logcat.read_text(encoding='utf-8', errors='replace'), pids)
    if failures:
        raise SystemExit('FAIL: application crash evidence:\n'+'\n'.join(failures))
    print('PASS: installed APK remained alive without application crashes')


if __name__ == '__main__':
    main()
