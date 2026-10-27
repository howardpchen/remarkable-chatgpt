#!/usr/bin/env python3
"""Reject duplicate generated XOVI pass-1 entries before a device load."""
import ast
import collections
import pathlib
import re
import sys
source = pathlib.Path(sys.argv[1]).read_text()
match = re.search(r'const char \*LINKTABLENAMES = ("[^"\n]*");', source)
if not match:
    raise SystemExit("Missing generated XOVI link table")
entries = [entry for entry in ast.literal_eval(match.group(1)).split("\0") if entry]
duplicates = [entry for entry, count in collections.Counter(entries).items() if count > 1]
if duplicates:
    raise SystemExit("Duplicate XOVI pass-1 entries: " + ", ".join(duplicates))
print("PASS: generated XOVI link entries unique")
