#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Build a Xiaomi R4/RD05 full 16 MiB SPI-NOR test image.

This tool keeps the device-specific bootloader, U-Boot environment, Xiaomi
BData, MAC/calibration areas and other non-firmware regions from a user-provided
full SPI dump, then replaces only the kernel/rootfs firmware regions.

It intentionally refuses to operate without a 16 MiB template dump. Do not use a
fullflash image generated from another router's dump.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys

FLASH_SIZE = 0x1000000
KERNEL_OFF = 0x00060000
ROOTFS_OFF = 0x00350000
ROOTFS_DATA_OFF = 0x00E60000


def sha256(data: bytes | bytearray) -> str:
    h = hashlib.sha256()
    h.update(data)
    return h.hexdigest()


def read_file(path: Path, label: str) -> bytes:
    if not path.exists():
        raise SystemExit(f"ERROR: {label} missing: {path}")
    return path.read_bytes()


def write_region(image: bytearray, offset: int, limit: int, payload: bytes, label: str) -> None:
    end = offset + len(payload)
    if end > limit:
        raise SystemExit(
            f"ERROR: {label} too large: start=0x{offset:x} size=0x{len(payload):x} "
            f"end=0x{end:x} limit=0x{limit:x}"
        )
    image[offset:end] = payload


def main() -> int:
    ap = argparse.ArgumentParser(description="Create RD05 full SPI-NOR image from template + OpenWrt kernel/rootfs")
    ap.add_argument("--template", required=True, type=Path, help="16 MiB original RD05 SPI dump/template")
    ap.add_argument("--kernel", required=True, type=Path, help="RD05 kernel-cs6c.bin")
    ap.add_argument("--rootfs", required=True, type=Path, help="RD05 rootfs.bin")
    ap.add_argument("--output", required=True, type=Path, help="output fullflash image")
    ap.add_argument("--kernel-offset", type=lambda x: int(x, 0), default=KERNEL_OFF)
    ap.add_argument("--rootfs-offset", type=lambda x: int(x, 0), default=ROOTFS_OFF)
    ap.add_argument("--rootfs-data-offset", type=lambda x: int(x, 0), default=ROOTFS_DATA_OFF)
    ap.add_argument(
        "--keep-rootfs-data",
        action="store_true",
        help="preserve template data after rootfs-data offset instead of erasing it to 0xff",
    )
    args = ap.parse_args()

    template = bytearray(read_file(args.template, "template SPI dump"))
    kernel = read_file(args.kernel, "kernel image")
    rootfs = read_file(args.rootfs, "rootfs image")

    if len(template) != FLASH_SIZE:
        raise SystemExit(
            f"ERROR: template must be exactly 16 MiB / 0x{FLASH_SIZE:x} bytes; "
            f"got 0x{len(template):x} bytes from {args.template}"
        )

    if kernel[:4] != b"cs6c":
        raise SystemExit(
            f"ERROR: kernel image does not start with cs6c header: {args.kernel}. "
            "Use the RD05 kernel-cs6c image path."
        )

    if rootfs[:4] != b"hsqs":
        raise SystemExit(
            f"ERROR: rootfs image does not start with SquashFS magic 'hsqs': {args.rootfs}"
        )

    if args.kernel_offset >= args.rootfs_offset:
        raise SystemExit("ERROR: kernel offset must be below rootfs offset")
    if args.rootfs_offset >= args.rootfs_data_offset:
        raise SystemExit("ERROR: rootfs offset must be below rootfs_data offset")
    if args.rootfs_data_offset > FLASH_SIZE:
        raise SystemExit("ERROR: rootfs_data offset is outside flash")

    # Erase firmware regions in the template copy before inserting OpenWrt.
    # Preserve 0x000000..0x060000: bootcode, env, bdata/device data, factory-like data.
    template[args.kernel_offset:args.rootfs_offset] = b"\xff" * (args.rootfs_offset - args.kernel_offset)
    template[args.rootfs_offset:args.rootfs_data_offset] = b"\xff" * (args.rootfs_data_offset - args.rootfs_offset)
    if not args.keep_rootfs_data:
        template[args.rootfs_data_offset:FLASH_SIZE] = b"\xff" * (FLASH_SIZE - args.rootfs_data_offset)

    write_region(template, args.kernel_offset, args.rootfs_offset, kernel, "kernel")
    write_region(template, args.rootfs_offset, args.rootfs_data_offset, rootfs, "rootfs")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(template)

    print("rtl8197f-rd05-fullflash:")
    print(f"  template:        {args.template}")
    print(f"  kernel:          {args.kernel} size=0x{len(kernel):x}")
    print(f"  rootfs:          {args.rootfs} size=0x{len(rootfs):x}")
    print(f"  output:          {args.output}")
    print(f"  kernel offset:   0x{args.kernel_offset:08x}")
    print(f"  rootfs offset:   0x{args.rootfs_offset:08x}")
    print(f"  rootfs_data off: 0x{args.rootfs_data_offset:08x}")
    print(f"  output sha256:   {sha256(template)}")
    if not args.keep_rootfs_data:
        print("  rootfs_data:     erased to 0xff")
    return 0


if __name__ == "__main__":
    sys.exit(main())
