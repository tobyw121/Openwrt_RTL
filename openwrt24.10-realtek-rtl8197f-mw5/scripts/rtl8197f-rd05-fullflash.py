#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Build a personalized Xiaomi R4/RD05 16 MiB SPI-NOR image.

Only kernel/rootfs regions are replaced. Bootloader, environment, BData,
MAC addresses and factory/RF calibration before 0x60000 remain byte-identical.
The output contains private device data and must not be published.
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
RD05_MARKER = b"model=RD05"


def sha256(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()


def read_file(path: Path, label: str) -> bytes:
    if not path.is_file():
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
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--template", required=True, type=Path, help="16 MiB original RD05 SPI dump")
    ap.add_argument("--kernel", required=True, type=Path, help="RD05 kernel-cs6c image")
    ap.add_argument("--rootfs", required=True, type=Path, help="RD05 SquashFS rootfs")
    ap.add_argument("--output", required=True, type=Path, help="output fullflash image")
    ap.add_argument("--manifest", type=Path, help="optional manifest output")
    ap.add_argument("--kernel-offset", type=lambda x: int(x, 0), default=KERNEL_OFF)
    ap.add_argument("--rootfs-offset", type=lambda x: int(x, 0), default=ROOTFS_OFF)
    ap.add_argument("--rootfs-data-offset", type=lambda x: int(x, 0), default=ROOTFS_DATA_OFF)
    ap.add_argument("--keep-rootfs-data", action="store_true")
    args = ap.parse_args()

    original = read_file(args.template, "template SPI dump")
    kernel = read_file(args.kernel, "kernel image")
    rootfs = read_file(args.rootfs, "rootfs image")

    if len(original) != FLASH_SIZE:
        raise SystemExit(
            f"ERROR: RD05 template must be exactly 16 MiB / 0x{FLASH_SIZE:x}; "
            f"got 0x{len(original):x}"
        )
    if RD05_MARKER not in original:
        raise SystemExit("ERROR: template lacks 'model=RD05'; refusing a foreign 16 MiB dump")
    if kernel[:4] != b"cs6c":
        raise SystemExit("ERROR: RD05 kernel image must start with the cs6c header")
    if rootfs[:4] != b"hsqs":
        raise SystemExit("ERROR: RD05 rootfs must start with SquashFS magic 'hsqs'")
    if not (0 < args.kernel_offset < args.rootfs_offset < args.rootfs_data_offset <= FLASH_SIZE):
        raise SystemExit("ERROR: invalid RD05 flash offset ordering")

    image = bytearray(original)
    image[args.kernel_offset:args.rootfs_offset] = b"\xff" * (args.rootfs_offset - args.kernel_offset)
    image[args.rootfs_offset:args.rootfs_data_offset] = b"\xff" * (args.rootfs_data_offset - args.rootfs_offset)
    if not args.keep_rootfs_data:
        image[args.rootfs_data_offset:FLASH_SIZE] = b"\xff" * (FLASH_SIZE - args.rootfs_data_offset)

    write_region(image, args.kernel_offset, args.rootfs_offset, kernel, "kernel")
    write_region(image, args.rootfs_offset, args.rootfs_data_offset, rootfs, "rootfs")

    if len(image) != FLASH_SIZE:
        raise SystemExit("ERROR: internal error: output size changed")
    if image[:args.kernel_offset] != original[:args.kernel_offset]:
        raise SystemExit("ERROR: RD05 boot/factory area changed unexpectedly")
    if args.keep_rootfs_data and image[args.rootfs_data_offset:] != original[args.rootfs_data_offset:]:
        raise SystemExit("ERROR: requested preserved rootfs_data changed unexpectedly")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(image)

    lines = [
        "Xiaomi R4/RD05 personalized SPI fullflash manifest",
        "WARNING=contains private device data; do not publish",
        f"template={args.template}",
        f"template_size={len(original)}",
        f"template_sha256={sha256(original)}",
        f"kernel={args.kernel}",
        f"kernel_size={len(kernel)}",
        f"kernel_sha256={sha256(kernel)}",
        f"rootfs={args.rootfs}",
        f"rootfs_size={len(rootfs)}",
        f"rootfs_sha256={sha256(rootfs)}",
        f"output={args.output}",
        f"output_size={len(image)}",
        f"output_sha256={sha256(image)}",
        f"kernel_offset=0x{args.kernel_offset:x}",
        f"rootfs_offset=0x{args.rootfs_offset:x}",
        f"rootfs_data_offset=0x{args.rootfs_data_offset:x}",
        f"preserved_boot_factory_sha256={sha256(image[:args.kernel_offset])}",
        f"rootfs_data_preserved={str(args.keep_rootfs_data).lower()}",
    ]
    if args.manifest:
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        args.manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print("rtl8197f-rd05-fullflash:")
    print(f"  template:        {args.template} sha256={sha256(original)}")
    print(f"  kernel/rootfs:   0x{len(kernel):x}/0x{len(rootfs):x}")
    print(f"  output:          {args.output} size=0x{len(image):x}")
    print(f"  output sha256:   {sha256(image)}")
    print("  preserved:       bootloader, environment, BData, MAC and factory/RF data")
    print(f"  rootfs_data:     {'preserved' if args.keep_rootfs_data else 'erased to 0xff'}")
    if args.manifest:
        print(f"  manifest:        {args.manifest}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
