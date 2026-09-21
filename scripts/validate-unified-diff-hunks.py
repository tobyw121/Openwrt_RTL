#!/usr/bin/env python3
"""Validate unified-diff hunk line counts before OpenWrt applies patches."""
from __future__ import annotations
import re
import sys
from pathlib import Path

HUNK = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")


def patch_files(root: Path, args: list[str]) -> list[Path]:
    if args:
        out: list[Path] = []
        for arg in args:
            p = (root / arg) if not Path(arg).is_absolute() else Path(arg)
            if p.is_dir():
                out.extend(sorted(p.rglob("*.patch")))
            elif p.is_file():
                out.append(p)
            else:
                print(f"missing patch path: {p}", file=sys.stderr)
                raise SystemExit(2)
        return sorted(dict.fromkeys(out))
    paths = [root / "target/linux/realtek/patches-6.6"]
    return [p for d in paths if d.is_dir() for p in sorted(d.glob("*.patch"))]


def validate(path: Path) -> list[str]:
    lines = path.read_text(errors="replace").splitlines()
    errors: list[str] = []
    i = 0
    hunks = 0
    while i < len(lines):
        m = HUNK.match(lines[i])
        if not m:
            i += 1
            continue
        hunks += 1
        old_expected = int(m.group(2) or "1")
        new_expected = int(m.group(4) or "1")
        old_seen = new_seen = 0
        header_line = i + 1
        i += 1
        while i < len(lines):
            line = lines[i]
            if HUNK.match(line) or line.startswith("diff --git "):
                break
            if line.startswith("--- ") and old_seen >= old_expected and new_seen >= new_expected:
                break
            if line.startswith("\\ No newline at end of file"):
                i += 1
                continue
            if not line:
                # A valid hunk line representing an empty context line still has
                # a prefix. A truly empty physical line terminates malformed input.
                break
            prefix = line[0]
            if prefix == " ":
                old_seen += 1
                new_seen += 1
            elif prefix == "-":
                old_seen += 1
            elif prefix == "+":
                new_seen += 1
            else:
                break
            i += 1
            if old_seen == old_expected and new_seen == new_expected:
                break
        if old_seen != old_expected or new_seen != new_expected:
            errors.append(
                f"{path}:{header_line}: hunk count mismatch: "
                f"old {old_seen}/{old_expected}, new {new_seen}/{new_expected}"
            )
    if hunks == 0:
        errors.append(f"{path}: no unified-diff hunks found")
    return errors


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    files = patch_files(root, sys.argv[1:])
    errors: list[str] = []
    for p in files:
        errors.extend(validate(p))
    if errors:
        print("Unified diff hunk validation FAILED:")
        for err in errors:
            print(f"  {err}")
        return 1
    print(f"Unified diff hunk validation OK: {len(files)} patches")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
