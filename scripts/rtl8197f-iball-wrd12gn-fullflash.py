#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build a personalized iBall WRD12GN 8 MiB SPI image.

Only 0x030000..0x790000 is replaced.  The bootloader, H601 factory/RF data,
configuration and database partitions come from the owner's own backup and
are preserved byte-for-byte.
"""
from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

FLASH_SIZE = 0x800000
FIRMWARE_OFFSET = 0x030000
FIRMWARE_END = 0x790000
FIRMWARE_SIZE = FIRMWARE_END - FIRMWARE_OFFSET
FACTORY_OFFSET = 0x020000
STOCK_ROOTFS_OFFSET = 0x230000
LOAD_ADDR = 0x80A00000


def sha256(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()


def sum16_be(data: bytes) -> int:
    total = 0
    for pos in range(0, len(data) & ~1, 2):
        total = (total + ((data[pos] << 8) | data[pos + 1])) & 0xFFFF
    if len(data) & 1:
        total = (total + (data[-1] << 8)) & 0xFFFF
    return total


def parse_firmware(blob: bytes, label: str, require_rootfs: bool) -> tuple[int, int]:
    if len(blob) < 16:
        raise SystemExit(f"ERROR: {label} is shorter than IMG_HEADER_T")
    signature, load, burn, length = struct.unpack_from(">4sIII", blob)
    if signature != b"cr6c":
        raise SystemExit(f"ERROR: {label} signature is {signature!r}, expected b'cr6c'")
    if load != LOAD_ADDR or burn != FIRMWARE_OFFSET:
        raise SystemExit(
            f"ERROR: {label} load/burn is 0x{load:08x}/0x{burn:08x}, "
            f"expected 0x{LOAD_ADDR:08x}/0x{FIRMWARE_OFFSET:08x}"
        )
    rootfs = 16 + length
    if rootfs > len(blob) or sum16_be(blob[16:rootfs]):
        raise SystemExit(f"ERROR: {label} kernel length/checksum is invalid")
    if require_rootfs and blob[rootfs:rootfs + 4] != b"hsqs":
        raise SystemExit(f"ERROR: {label} has no SquashFS at header-derived offset 0x{rootfs:x}")
    if require_rootfs:
        if rootfs + 96 > len(blob):
            raise SystemExit(f"ERROR: {label} has a truncated SquashFS superblock")
        bytes_used = struct.unpack_from("<Q", blob, rootfs + 40)[0]
        checksum_offset = (bytes_used + 0xFFF) & ~0xFFF
        checksum_end = rootfs + checksum_offset + 2
        if bytes_used < 96 or checksum_end > len(blob):
            raise SystemExit(f"ERROR: {label} has an invalid SquashFS length/trailer")
        marker = struct.unpack_from(">I", blob, rootfs + 8)[0]
        if marker != checksum_offset + 2 - 0x282:
            raise SystemExit(f"ERROR: {label} has an invalid Realtek rootfs length marker")
        if any(blob[rootfs + bytes_used:rootfs + checksum_offset]):
            raise SystemExit(f"ERROR: {label} rootfs checksum gap is not zero-filled")
        if sum16_be(blob[rootfs:checksum_end]):
            raise SystemExit(f"ERROR: {label} Realtek rootfs checksum is invalid")
    return length, rootfs


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--template", required=True, type=Path)
    parser.add_argument("--firmware", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--manifest", type=Path)
    args = parser.parse_args()

    template = args.template.read_bytes()
    firmware = args.firmware.read_bytes()
    if len(template) != FLASH_SIZE:
        parser.error(f"template must be exactly 0x{FLASH_SIZE:x} bytes")
    if len(firmware) > FIRMWARE_SIZE:
        parser.error(f"firmware is 0x{len(firmware):x}; maximum is 0x{FIRMWARE_SIZE:x}")
    if template[FACTORY_OFFSET:FACTORY_OFFSET + 4] != b"H601":
        raise SystemExit("ERROR: template lacks WRD12GN H601 factory marker")
    if template[STOCK_ROOTFS_OFFSET:STOCK_ROOTFS_OFFSET + 4] != b"hsqs":
        raise SystemExit("ERROR: template lacks stock WRD12GN SquashFS at 0x230000")

    template_len, _ = parse_firmware(
        template[FIRMWARE_OFFSET:STOCK_ROOTFS_OFFSET], "stock WRD12GN firmware", False
    )
    firmware_len, rootfs = parse_firmware(firmware, "OpenWrt WRD12GN firmware", True)

    output = bytearray(template)
    output[FIRMWARE_OFFSET:FIRMWARE_END] = b"\xff" * FIRMWARE_SIZE
    output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)] = firmware
    if output[:FIRMWARE_OFFSET] != template[:FIRMWARE_OFFSET]:
        raise SystemExit("ERROR: boot/factory region changed")
    if output[FIRMWARE_END:] != template[FIRMWARE_END:]:
        raise SystemExit("ERROR: config/database region changed")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    if args.output.read_bytes() != output:
        raise SystemExit("ERROR: output readback mismatch")

    lines = [
        "iBall WRD12GN personalized SPI fullflash manifest v44.63",
        "WARNING=contains private device data; do not publish",
        f"template_sha256={sha256(template)}",
        f"template_kernel_len=0x{template_len:x}",
        f"firmware_sha256={sha256(firmware)}",
        f"firmware_size=0x{len(firmware):x}",
        f"firmware_kernel_len=0x{firmware_len:x}",
        f"firmware_rootfs_offset=0x{rootfs:x}",
        f"output_sha256={sha256(output)}",
        "preserved=0x000000..0x030000,0x790000..0x800000",
    ]
    if args.manifest:
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        args.manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("WRD12GN fullflash: firmware replaced, private regions preserved")
    print(f"output sha256: {sha256(output)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
