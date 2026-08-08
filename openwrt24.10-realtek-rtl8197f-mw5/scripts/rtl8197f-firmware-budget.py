#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Strict size gate for RTL8197F combined kernel/rootfs images.

OpenWrt's generic check-size deliberately removes oversized files but returns
success, which causes confusing follow-up errors in private full-flash recipes.
This tool predicts the erase-block-aligned size before pad-rootfs and exits
non-zero without deleting the image when the fixed OEM firmware window cannot
hold it.
"""
from __future__ import annotations

import argparse
from pathlib import Path

UNITS = {
    "": 1,
    "b": 1,
    "k": 1024,
    "kb": 1024,
    "kib": 1024,
    "m": 1024 * 1024,
    "mb": 1024 * 1024,
    "mib": 1024 * 1024,
}


def parse_size(value: str) -> int:
    text = value.strip().lower()
    if not text:
        raise argparse.ArgumentTypeError("empty size")
    split = len(text)
    while split and text[split - 1].isalpha():
        split -= 1
    number, suffix = text[:split], text[split:]
    if suffix not in UNITS:
        raise argparse.ArgumentTypeError(f"unsupported size suffix in {value!r}")
    try:
        base = int(number, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"invalid size {value!r}") from exc
    if base <= 0:
        raise argparse.ArgumentTypeError("size must be positive")
    return base * UNITS[suffix]


def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) // alignment * alignment


def fmt(value: int) -> str:
    return f"0x{value:x} ({value} bytes, {value / 1024:.1f} KiB)"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", required=True, type=Path)
    parser.add_argument("--limit", required=True, type=parse_size)
    parser.add_argument("--erase-size", required=True, type=parse_size)
    parser.add_argument("--label", default="rtl8197f")
    args = parser.parse_args()

    if not args.image.is_file():
        parser.error(f"image does not exist: {args.image}")
    if args.erase_size & (args.erase_size - 1):
        parser.error("erase size must be a power of two")

    raw = args.image.stat().st_size
    padded = align_up(raw, args.erase_size)
    free_raw = args.limit - raw
    free_padded = args.limit - padded

    if padded > args.limit:
        over = padded - args.limit
        print(f"ERROR: {args.label} firmware exceeds its fixed SPI window")
        print(f"  image:       {args.image}")
        print(f"  raw size:    {fmt(raw)}")
        print(f"  erase size:  {fmt(args.erase_size)}")
        print(f"  padded size: {fmt(padded)}")
        print(f"  limit:       {fmt(args.limit)}")
        print(f"  over budget: {fmt(over)}")
        print("  The image was left intact; CFM/LOG/ENV were not touched.")
        return 1

    print(f"rtl8197f-firmware-budget: {args.label}")
    print(f"  raw={fmt(raw)} padded={fmt(padded)} limit={fmt(args.limit)}")
    print(f"  free-before-pad={fmt(free_raw)} free-after-pad={fmt(free_padded)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
