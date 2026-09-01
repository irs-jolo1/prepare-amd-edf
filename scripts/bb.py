#!/usr/bin/env python3
"""bb - a thin wrapper around bitbake.

Usage:
    bb <bitbake args...>
    bb search <regex> -- <bitbake args...>

Plain invocations are passed straight through to `bitbake`, unmodified,
with a real tty attached (interactive output, colors, progress bars, etc.
all behave exactly as if you'd run bitbake directly):

    bb -e core-image-minimal-initramfs
        -> bitbake -e core-image-minimal-initramfs

`bb search` is roughly equivalent to piping bitbake's output through
grep, except that matching lines whose value is a long, whitespace
separated list (e.g. RDEPENDS, RRECOMMENDS, DEPENDS, ...) are
pretty-printed one item per line instead of being dumped as a single
wall of text:

    bb search RDEPENDS -- -e core-image-minimal-initramfs
        -> bitbake -e core-image-minimal-initramfs | grep RDEPENDS
           (with matching lines reformatted for readability)
"""

import os
import re
import shutil
import subprocess
import sys

# Matches lines like:
#   RDEPENDS="foo bar baz"
#   RDEPENDS:pkg-name="foo bar baz"
#   FOO:append = "foo bar baz"
ASSIGNMENT_RE = re.compile(
    r'^(?P<indent>\s*)'
    r'(?P<name>[A-Za-z_][A-Za-z0-9_:.\-\[\]]*)'
    r'(?P<op>\s*[:+.]?=\s*)'
    r'"(?P<value>.*)"\s*$'
)

# Only reformat if the value looks like a space separated list with more
# than this many items, otherwise leave short/simple assignments alone.
MIN_ITEMS_TO_WRAP = 4


class Color:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    CYAN = "\033[36m"
    YELLOW = "\033[33m"
    RED_BOLD = "\033[1;31m"

    def __init__(self, enabled: bool):
        self.enabled = enabled

    def __call__(self, code: str, text: str) -> str:
        if not self.enabled:
            return text
        return f"{code}{text}{self.RESET}"


def use_color() -> bool:
    if os.environ.get("NO_COLOR"):
        return False
    if os.environ.get("FORCE_COLOR"):
        return True
    return sys.stdout.isatty()


def highlight_matches(text: str, pattern: "re.Pattern[str]", color: Color) -> str:
    if not color.enabled:
        return text
    return pattern.sub(lambda m: color(Color.RED_BOLD, m.group(0)), text)


def format_line(line: str, color: Color) -> str:
    match = ASSIGNMENT_RE.match(line)
    if not match:
        return line

    name = match.group("name")
    indent = match.group("indent")
    value = match.group("value")

    items = value.split()
    if len(items) < MIN_ITEMS_TO_WRAP:
        return line

    quote = color(Color.YELLOW, '"')

    lines = [f'{indent}{name} = {quote}']
    for item in items:
        lines.append(f'{indent}    {item}')
    lines.append(f'{indent}{quote}')
    return "\n".join(lines)


def run_passthrough(argv):
    """Run bitbake directly, replacing this process entirely.

    This keeps bitbake attached to the real tty (if any), so interactive
    behavior (colors, progress, prompts) works exactly as if bitbake had
    been invoked directly, with no buffering delays.
    """
    bitbake = shutil.which("bitbake") or "bitbake"
    os.execvp(bitbake, [bitbake] + argv)


def run_search(argv):
    if not argv:
        print("usage: bb search <regex> -- <bitbake args...>", file=sys.stderr)
        return 2

    regex = argv[0]
    rest = argv[1:]
    if rest and rest[0] == "--":
        rest = rest[1:]
    bitbake_args = rest

    pattern = re.compile(regex)
    color = Color(use_color())

    cmd = ["bitbake"] + bitbake_args

    # bitbake fully buffers its output when stdout isn't a tty (which is the
    # case here, since we read it through a pipe), so output would otherwise
    # only show up in large, delayed chunks. `stdbuf` forces line buffering
    # so we can stream matches live.
    if shutil.which("stdbuf"):
        cmd = ["stdbuf", "-oL", "-eL"] + cmd

    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    assert proc.stdout is not None
    for line in proc.stdout:
        line = line.rstrip("\n")
        if pattern.search(line):
            formatted = format_line(line, color)
            formatted = highlight_matches(formatted, pattern, color)
            print(formatted, flush=True)

    return proc.wait()


def main(argv):
    if argv and argv[0] == "search":
        return run_search(argv[1:])
    return run_passthrough(argv)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
