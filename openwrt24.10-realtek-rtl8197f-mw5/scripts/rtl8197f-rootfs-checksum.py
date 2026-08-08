#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build and verify the Realtek/Tenda SquashFS trailer used by RTL8197F boot code.

The Tenda MW5 bootloader performs two non-standard operations before booting:

* it reads a big-endian 32-bit length marker from SquashFS superblock offset
  ``+0x08`` (the normal SquashFS-4 ``mkfs_time`` field), adds ``0x282``, and
  checks exactly that many rootfs bytes;
* it expects a final big-endian 16-bit additive checksum word after a 4 KiB
  aligned SquashFS image, making the modulo-65536 word sum equal to zero.

OpenWrt's stock SquashFS image contains a timestamp at ``+0x08``.  Without the
MW5 marker the bootloader interprets that timestamp as a multi-gigabyte length
and appears to hang forever after printing ``irq:...``.  This tool writes both
the proprietary marker and checksum while retaining the normal SquashFS-4
``bytes_used`` field at offset ``+0x28`` for Linux 6.6.
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

SQUASHFS_MAGIC = b"hsqs"
SQUASHFS_BOOT_MARKER_OFFSET = 8
SQUASHFS_BYTES_USED_OFFSET = 40
SQUASHFS_SUPERBLOCK_SIZE = 96
CHECKSUM_SIZE = 2
ALIGNMENT = 4096
BOOTLOADER_LENGTH_BIAS = 0x282


@dataclass(frozen=True)
class RootfsInfo:
    bytes_used: int
    original_bytes_used: int
    checksum_offset: int
    checksum_word: int
    checksum_sum: int
    total_size: int
    bootloader_marker: int
    bootloader_check_length: int


def align_up(value: int, alignment: int = ALIGNMENT) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def sum16_be(data: bytes | bytearray | memoryview) -> int:
    """Return modulo-65536 sum of big-endian 16-bit words."""
    total = 0
    even = len(data) & ~1
    for pos in range(0, even, 2):
        total = (total + ((data[pos] << 8) | data[pos + 1])) & 0xFFFF
    if len(data) & 1:
        total = (total + (data[-1] << 8)) & 0xFFFF
    return total


def parse_superblock(data: bytes | bytearray, label: str) -> int:
    if len(data) < SQUASHFS_SUPERBLOCK_SIZE:
        raise ValueError(f"{label}: shorter than SquashFS superblock")
    if data[:4] != SQUASHFS_MAGIC:
        raise ValueError(f"{label}: missing little-endian SquashFS magic 'hsqs'")

    major, minor = struct.unpack_from("<HH", data, 28)
    if major != 4:
        raise ValueError(f"{label}: unsupported SquashFS version {major}.{minor}; expected 4.x")

    bytes_used = struct.unpack_from("<Q", data, SQUASHFS_BYTES_USED_OFFSET)[0]
    if bytes_used < SQUASHFS_SUPERBLOCK_SIZE:
        raise ValueError(f"{label}: implausible bytes_used=0x{bytes_used:x}")
    return bytes_used


def expected_boot_marker(checksum_end: int) -> int:
    if checksum_end < BOOTLOADER_LENGTH_BIAS:
        raise ValueError(
            f"rootfs checksum length 0x{checksum_end:x} is smaller than "
            f"MW5 bootloader bias 0x{BOOTLOADER_LENGTH_BIAS:x}"
        )
    marker = checksum_end - BOOTLOADER_LENGTH_BIAS
    if marker > 0xFFFFFFFF:
        raise ValueError(f"MW5 bootloader marker does not fit in 32 bits: 0x{marker:x}")
    return marker


def build_rootfs(source: bytes, label: str = "input rootfs") -> tuple[bytes, RootfsInfo]:
    old_bytes_used = parse_superblock(source, label)
    if old_bytes_used > len(source):
        raise ValueError(
            f"{label}: bytes_used 0x{old_bytes_used:x} exceeds file size 0x{len(source):x}"
        )

    trailing = source[old_bytes_used:]
    if any(trailing):
        raise ValueError(
            f"{label}: non-zero data follows bytes_used; refusing an ambiguous rootfs"
        )

    output = bytearray(source[:old_bytes_used])

    # Keep the SquashFS superblock length standards-compliant.  The OEM
    # checksum is an external trailer and must not be counted in bytes_used.
    # Extending bytes_used by two makes Linux 6.6 reject the final ID index
    # table because next_table - id_table_start becomes 10 instead of 8.
    checksum_offset = align_up(old_bytes_used)
    checksum_end = checksum_offset + CHECKSUM_SIZE
    boot_marker = expected_boot_marker(checksum_end)

    # The OEM MW5 bootloader reads this as a big-endian 32-bit length marker.
    # Linux SquashFS treats it only as mkfs_time, so filesystem mounting remains
    # standards-compatible even though the displayed creation time is synthetic.
    struct.pack_into(">I", output, SQUASHFS_BOOT_MARKER_OFFSET, boot_marker)

    if checksum_offset > len(output):
        output.extend(b"\x00" * (checksum_offset - len(output)))

    checksum_word = (-sum16_be(output)) & 0xFFFF
    output.extend(struct.pack(">H", checksum_word))
    checksum_sum = sum16_be(output)
    if checksum_sum != 0:
        raise RuntimeError(
            f"internal checksum error: sum=0x{checksum_sum:04x}, expected 0x0000"
        )

    info = RootfsInfo(
        bytes_used=old_bytes_used,
        original_bytes_used=old_bytes_used,
        checksum_offset=checksum_offset,
        checksum_word=checksum_word,
        checksum_sum=checksum_sum,
        total_size=len(output),
        bootloader_marker=boot_marker,
        bootloader_check_length=checksum_end,
    )
    return bytes(output), info


def check_rootfs(data: bytes, label: str = "rootfs", allow_trailing: bool = True) -> RootfsInfo:
    bytes_used = parse_superblock(data, label)
    original_bytes_used = bytes_used
    checksum_offset = align_up(bytes_used)
    checksum_end = checksum_offset + CHECKSUM_SIZE
    if checksum_end > len(data):
        raise ValueError(
            f"{label}: checksum at 0x{checksum_offset:x} exceeds available size 0x{len(data):x}"
        )
    if not allow_trailing and checksum_end != len(data):
        raise ValueError(
            f"{label}: unexpected trailing data after checksum: "
            f"checksum end 0x{checksum_end:x}, file size 0x{len(data):x}"
        )

    marker = struct.unpack_from(">I", data, SQUASHFS_BOOT_MARKER_OFFSET)[0]
    expected_marker = expected_boot_marker(checksum_end)
    if marker != expected_marker:
        interpreted = marker + BOOTLOADER_LENGTH_BIAS
        raise ValueError(
            f"{label}: invalid MW5 bootloader length marker at superblock+0x08: "
            f"0x{marker:08x}, expected 0x{expected_marker:08x}; bootloader would "
            f"check 0x{interpreted:x} bytes instead of 0x{checksum_end:x}"
        )

    checked = data[:checksum_end]
    checksum_sum = sum16_be(checked)
    checksum_word = struct.unpack_from(">H", data, checksum_offset)[0]
    if checksum_sum != 0:
        raise ValueError(
            f"{label}: invalid Realtek rootfs checksum at 0x{checksum_offset:x}: "
            f"word=0x{checksum_word:04x}, sum=0x{checksum_sum:04x}"
        )

    if any(data[bytes_used:checksum_offset]):
        raise ValueError(f"{label}: non-zero data in checksum alignment padding")

    return RootfsInfo(
        bytes_used=bytes_used,
        original_bytes_used=original_bytes_used,
        checksum_offset=checksum_offset,
        checksum_word=checksum_word,
        checksum_sum=checksum_sum,
        total_size=checksum_end,
        bootloader_marker=marker,
        bootloader_check_length=checksum_end,
    )


def print_info(prefix: str, info: RootfsInfo) -> None:
    print(f"{prefix}: bytes_used=0x{info.bytes_used:x}")
    print(f"{prefix}: original_bytes_used=0x{info.original_bytes_used:x}")
    print(f"{prefix}: checksum_offset=0x{info.checksum_offset:x}")
    print(f"{prefix}: checksum_word=0x{info.checksum_word:04x}")
    print(f"{prefix}: checksum_sum=0x{info.checksum_sum:04x}")
    print(f"{prefix}: total_size=0x{info.total_size:x}")
    print(f"{prefix}: bootloader_marker=0x{info.bootloader_marker:08x}")
    print(f"{prefix}: bootloader_check_length=0x{info.bootloader_check_length:x}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    build = sub.add_parser("build", help="add MW5 boot marker and checksum trailer")
    build.add_argument("input", type=Path)
    build.add_argument("output", type=Path)

    check = sub.add_parser("check", help="verify an existing checksummed MW5 rootfs")
    check.add_argument("input", type=Path)
    check.add_argument(
        "--exact-size",
        action="store_true",
        help="reject bytes after the checksum word",
    )

    args = parser.parse_args(argv)
    try:
        source = args.input.read_bytes()
        if args.command == "build":
            output, info = build_rootfs(source, str(args.input))
            verified = check_rootfs(output, str(args.output), allow_trailing=False)
            if verified != info:
                raise RuntimeError("internal build/check metadata mismatch")
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(output)
            print_info("rtl8197f-rootfs-checksum build", info)
        else:
            info = check_rootfs(
                source,
                str(args.input),
                allow_trailing=not args.exact_size,
            )
            print_info("rtl8197f-rootfs-checksum check", info)
    except (OSError, ValueError, RuntimeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
