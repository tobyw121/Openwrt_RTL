#!/usr/bin/env python3
"""Compact MW5 runtime shell scripts without changing executable shell input.

Only full-line comments and empty lines outside here-documents are removed.
Here-document payloads are copied byte-for-byte. The source files in the tree
remain readable; this helper is run on the package staging copy only.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

HEREDOC_RE = re.compile(
    r"<<(?P<tabs>-)?[ \t]*(?P<quote>['\"]?)(?P<word>[A-Za-z_][A-Za-z0-9_]*)(?P=quote)"
)
COMMENT_RE = re.compile(r"^[ \t]*#")
BLANK_RE = re.compile(r"^[ \t]*(?:\r?\n)?$")


def compact(data: str) -> str:
    lines = data.splitlines(keepends=True)
    out: list[str] = []
    pending: list[tuple[str, bool]] = []

    for index, line in enumerate(lines):
        if pending:
            out.append(line)
            word, strip_tabs = pending[0]
            candidate = line.rstrip("\r\n")
            if strip_tabs:
                candidate = candidate.lstrip("\t")
            if candidate == word:
                pending.pop(0)
            continue

        # The interpreter line is executable metadata, not a comment.
        if index == 0 and line.startswith("#!"):
            out.append(line)
        elif COMMENT_RE.match(line) or BLANK_RE.match(line):
            continue
        else:
            out.append(line)

        # Here-doc bodies start on the following physical line. Preserve every
        # body byte, including lines that look like comments or blank lines.
        for match in HEREDOC_RE.finditer(line):
            pending.append((match.group("word"), bool(match.group("tabs"))))

    if pending:
        raise ValueError(f"unterminated here-document(s): {pending!r}")

    return "".join(out)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+")
    args = parser.parse_args()

    total_before = 0
    total_after = 0
    for name in args.files:
        path = Path(name)
        if not path.is_file():
            print(f"mw5-shell-compact: missing file: {path}", file=sys.stderr)
            return 2
        raw = path.read_text(encoding="utf-8")
        compacted = compact(raw)
        before = len(raw.encode("utf-8"))
        after = len(compacted.encode("utf-8"))
        total_before += before
        total_after += after
        path.write_text(compacted, encoding="utf-8")
        path.chmod(path.stat().st_mode | 0o111)
        print(f"mw5-shell-compact: {path.name}: {before} -> {after} bytes")

    print(
        f"mw5-shell-compact: total {total_before} -> {total_after} bytes "
        f"(saved {total_before - total_after})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
