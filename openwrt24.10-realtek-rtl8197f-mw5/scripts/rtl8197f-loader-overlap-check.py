#!/usr/bin/env python3
"""Reject RTL8197F in-place loader/kernel overlap before loader linking."""
import argparse
from pathlib import Path


def parse_size(text: str) -> int:
    units = {'k': 1024, 'kb': 1024, 'kib': 1024,
             'm': 1024**2, 'mb': 1024**2, 'mib': 1024**2}
    s = text.strip().lower()
    for suffix, mul in units.items():
        if s.endswith(suffix):
            return int(s[:-len(suffix)], 0) * mul
    return int(s, 0)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument('--lzma', required=True, type=Path)
    ap.add_argument('--kernel-loadaddr', required=True, type=lambda x: int(x, 0))
    ap.add_argument('--loader-start', required=True, type=lambda x: int(x, 0))
    ap.add_argument('--safety-margin', default='0', type=parse_size)
    ap.add_argument('--label', default='rtl8197f')
    args = ap.parse_args()

    data = args.lzma.read_bytes()
    if len(data) < 13:
        raise SystemExit(f'{args.label}: truncated LZMA-alone stream')
    out_size = int.from_bytes(data[5:13], 'little')
    if out_size in (0, 0xffffffffffffffff):
        raise SystemExit(f'{args.label}: LZMA stream has unknown/invalid output size')
    kernel_end = args.kernel_loadaddr + out_size
    limit = args.loader_start - args.safety_margin
    if kernel_end > limit:
        overlap = kernel_end - limit
        raise SystemExit(
            f'{args.label}: decompressed kernel/DTB overlaps loader safety window: '
            f'load=0x{args.kernel_loadaddr:08x} size=0x{out_size:x} '
            f'end=0x{kernel_end:08x} loader=0x{args.loader_start:08x} '
            f'margin=0x{args.safety_margin:x} excess=0x{overlap:x}')
    print(
        f'{args.label}: loader overlap check OK: '
        f'kernel_end=0x{kernel_end:08x} loader=0x{args.loader_start:08x} '
        f'margin=0x{args.safety_margin:x} free=0x{args.loader_start-kernel_end:x}')


if __name__ == '__main__':
    main()
