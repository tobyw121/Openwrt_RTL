#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Create an 8 MiB Tenda/Lynx RTL8197F full SPI-NOR test image.

The image is based on a user-provided original BH25Q64/SPI dump.  The script
preserves boot code, factory/calibration data and both Tenda configuration
copies, then replaces only the OpenWrt firmware region:

    0x000000..0x020000  boot, preserved
    0x020000..0x030000  factory/calibration, preserved
    0x030000..0x320000  kernel cr6c payload
    0x320000..0x7e0000  squashfs rootfs
    0x7e0000..0x800000  Tenda config/config_bak, preserved by default
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys

FLASH_SIZE = 0x00800000
KERNEL_OFF = 0x00030000
ROOTFS_OFF = 0x00320000
CONFIG_OFF = 0x007E0000


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


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--template", required=True, type=Path, help="8 MiB original Tenda/Lynx SPI dump")
    ap.add_argument("--kernel", required=True, type=Path, help="kernel-cr6c.bin")
    ap.add_argument("--rootfs", required=True, type=Path, help="squashfs rootfs")
    ap.add_argument("--output", required=True, type=Path, help="output fullflash image")
    ap.add_argument("--kernel-offset", type=lambda x: int(x, 0), default=KERNEL_OFF)
    ap.add_argument("--rootfs-offset", type=lambda x: int(x, 0), default=ROOTFS_OFF)
    ap.add_argument("--config-offset", type=lambda x: int(x, 0), default=CONFIG_OFF)
    ap.add_argument("--erase-config", action="store_true", help="erase 0x7e0000..end instead of preserving Tenda config")
    args = ap.parse_args(argv)

    template = bytearray(read_file(args.template, "template SPI dump"))
    kernel = read_file(args.kernel, "kernel image")
    rootfs = read_file(args.rootfs, "rootfs image")

    if len(template) != FLASH_SIZE:
        raise SystemExit(f"ERROR: template must be 8 MiB / 0x{FLASH_SIZE:x}; got 0x{len(template):x}")
    if kernel[:4] != b"cr6c":
        raise SystemExit("ERROR: kernel image must start with Realtek/Tenda cr6c header")
    if rootfs[:4] != b"hsqs":
        raise SystemExit("ERROR: rootfs image must start with SquashFS magic 'hsqs'")
    if not (args.kernel_offset < args.rootfs_offset < args.config_offset <= FLASH_SIZE):
        raise SystemExit("ERROR: invalid offset ordering")

    # Preserve 0x000000..kernel_offset and, unless requested otherwise, the two
    # Tenda config copies at the top of flash.  Replace firmware area only.
    template[args.kernel_offset:args.config_offset] = b"\xff" * (args.config_offset - args.kernel_offset)
    if args.erase_config:
        template[args.config_offset:FLASH_SIZE] = b"\xff" * (FLASH_SIZE - args.config_offset)

    write_region(template, args.kernel_offset, args.rootfs_offset, kernel, "kernel")
    write_region(template, args.rootfs_offset, args.config_offset, rootfs, "rootfs")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(template)
    print("rtl8197f-tenda-lynx-fullflash:")
    print(f"  template:      {args.template}")
    print(f"  kernel:        {args.kernel} size=0x{len(kernel):x}")
    print(f"  rootfs:        {args.rootfs} size=0x{len(rootfs):x}")
    print(f"  kernel offset: 0x{args.kernel_offset:08x}")
    print(f"  rootfs offset: 0x{args.rootfs_offset:08x}")
    print(f"  config offset: 0x{args.config_offset:08x} ({'erased' if args.erase_config else 'preserved'})")
    print(f"  output:        {args.output}")
    print(f"  sha256:        {sha256(template)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
