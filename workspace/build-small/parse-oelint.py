#!/usr/bin/env python3
"""Colorize oelint output from stdin.

Usage: oelint-adv ... | parse-oelint.py
"""

import os
import re
import sys
from collections import Counter, defaultdict

C = {
    "error":   "\033[1;31m", "warning": "\033[1;33m",
    "info":    "\033[1;36m", "debug":   "\033[1;35m",
    "rule":    "\033[35m",   "path":    "\033[34m",
    "bold":    "\033[1m",    "dim":     "\033[2m",
    "reset":   "\033[0m",
}
ICONS = {"error": "✗", "warning": "⚠", "info": "ℹ", "debug": "◇"}
ORDER = ["error", "warning", "info", "debug"]
BRANCH_RE = re.compile(r"\s*\[branch:[^\]]+\]\s*$")


def main():
    color = sys.stdout.isatty()
    def c(k, s): return f"{C[k]}{s}{C['reset']}" if color else s

    cwd = os.getcwd()
    grouped = defaultdict(lambda: defaultdict(list))
    counts = Counter()

    for line in sys.stdin:
        line = BRANCH_RE.sub("", line.rstrip())
        parts = line.split(":", 2)
        if len(parts) < 3 or parts[0] not in ICONS:
            continue
        level, rule, rest = parts
        msg, _, path = rest.rpartition(":")
        path = os.path.relpath(path.strip(), cwd) if path.startswith(cwd) else path.strip()
        counts[level] += 1
        grouped[level][path].append((rule, msg.strip()))

    for level in ORDER:
        if level not in grouped:
            continue
        print(c(level, f"{ICONS[level]} {level.upper()} ({counts[level]})"))
        for path, entries in sorted(grouped[level].items()):
            print(f"  {c('path', path)}")
            for rule, msg in entries:
                print(f"    {msg}  {c('rule', rule)}")
        print()

    if counts:
        print(c("bold", "Summary: ") + "  ".join(
            c(l, f"{ICONS[l]} {counts[l]} {l}") for l in ORDER if counts[l]))
    sys.exit(1 if counts.get("error") else 0)


if __name__ == "__main__":
    main()
