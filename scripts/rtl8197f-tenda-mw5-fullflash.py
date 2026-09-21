#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build a personalized Tenda Nova MW5 8 MiB SPI-NOR image.

The input firmware must be the combined OpenWrt ``firmware-cr6c.bin`` image.
Only the verified firmware window 0x030000..0x5c0000 is replaced. Everything
else is retained byte-for-byte from the user's own full SPI dump, including the
bootloader, factory/calibration/configuration areas, logs and ENV.

The generated image is device-specific and contains private data. Never use a
template from another router and never publish the resulting fullflash image.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from dataclasses import dataclass
from pathlib import Path

FLASH_SIZE = 0x800000
BOOTLOADER_OFFSET = 0x000000
BOOTLOADER_SIZE = 0x020000
CFG_OFFSET = 0x020000
CFG_SIZE = 0x010000
FIRMWARE_OFFSET = 0x030000
FIRMWARE_SIZE = 0x590000
CFM_OFFSET = 0x5C0000
CFM_SIZE = 0x010000
CFM_BACKUP_OFFSET = 0x5D0000
CFM_BACKUP_SIZE = 0x010000
LOG_OFFSET = 0x5E0000
LOG_SIZE = 0x210000
ENV_OFFSET = 0x7F0000
ENV_SIZE = 0x010000
LOGICAL_HEADER_SIZE = 16
SUPPORTED_SIGNATURES = (b"cr6c", b"cs6c", b"csys")
EXPECTED_LOADADDR = 0x80A00000
EXPECTED_BURNADDR = FIRMWARE_OFFSET
MW5_MODEL_MARKERS = (b"sys.model=MW5", b"Mw5;")


@dataclass(frozen=True)
class FirmwareInfo:
    signature: str
    load_addr: int
    burn_addr: int
    image_len: int
    rootfs_offset: int
    checksum_sum: int
    rootfs_bytes_used: int
    rootfs_checksum_offset: int
    rootfs_checksum_word: int
    rootfs_checksum_sum: int
    rootfs_bootloader_marker: int
    rootfs_bootloader_check_length: int


def sha256(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()


def sum16_be(data: bytes) -> int:
    """Return the folded big-endian 16-bit sum used by the bootloader."""
    total = 0
    even = len(data) & ~1
    for i in range(0, even, 2):
        total = (total + ((data[i] << 8) | data[i + 1])) & 0xFFFF
    if len(data) & 1:
        total = (total + (data[-1] << 8)) & 0xFFFF
    return total


def parse_firmware(
    blob: bytes, label: str, *, require_immediate_payload: bool = False
) -> FirmwareInfo:
    if len(blob) < LOGICAL_HEADER_SIZE:
        raise SystemExit(f"ERROR: {label} is shorter than the 16-byte IMG_HEADER_T")

    signature = blob[:4]
    if signature not in SUPPORTED_SIGNATURES:
        raise SystemExit(
            f"ERROR: {label} has unsupported signature {signature!r}; "
            "expected cr6c/cs6c/csys"
        )

    load_addr, burn_addr, image_len = struct.unpack_from(">III", blob, 4)
    if load_addr != EXPECTED_LOADADDR:
        raise SystemExit(
            f"ERROR: {label} load address is 0x{load_addr:08x}; "
            f"MW5 requires 0x{EXPECTED_LOADADDR:08x}"
        )
    if burn_addr != EXPECTED_BURNADDR:
        raise SystemExit(
            f"ERROR: {label} burn address is 0x{burn_addr:08x}; "
            f"MW5 requires 0x{EXPECTED_BURNADDR:08x}"
        )
    if image_len < 42:
        raise SystemExit(f"ERROR: {label} IMG_HEADER_T.len is implausibly small: {image_len}")

    rootfs_offset = LOGICAL_HEADER_SIZE + image_len
    if rootfs_offset + 4 > len(blob):
        raise SystemExit(
            f"ERROR: {label} header length points outside the file: "
            f"rootfs offset 0x{rootfs_offset:x}, file size 0x{len(blob):x}"
        )
    if blob[rootfs_offset:rootfs_offset + 4] != b"hsqs":
        raise SystemExit(
            f"ERROR: {label} has no little-endian SquashFS magic at the "
            f"header-derived offset 0x{rootfs_offset:x}"
        )

    if require_immediate_payload and blob[LOGICAL_HEADER_SIZE:LOGICAL_HEADER_SIZE + 40] == b"\x00" * 40:
        raise SystemExit(
            f"ERROR: {label} starts with the OEM 40-byte payload prefix after "
            "IMG_HEADER_T. The OpenWrt MW5 LZMA loader is linked at "
            "0x80a00000 and must start immediately at firmware+0x10; this "
            "layout would shift it to 0x80a00028 and hang before Linux output."
        )

    checksum_sum = sum16_be(blob[LOGICAL_HEADER_SIZE:rootfs_offset])
    if checksum_sum != 0:
        raise SystemExit(
            f"ERROR: {label} Realtek kernel checksum is invalid: "
            f"sum=0x{checksum_sum:04x}, expected 0x0000"
        )

    # Tenda's RTL8197F bootloader validates an additional checksum after the
    # SquashFS.  Keep bytes_used as the real SquashFS filesystem length; the
    # checksum is an external BE16 trailer at the next 4 KiB boundary.
    if rootfs_offset + 96 > len(blob):
        raise SystemExit(f"ERROR: {label} rootfs is shorter than a SquashFS superblock")
    major, minor = struct.unpack_from("<HH", blob, rootfs_offset + 28)
    if major != 4:
        raise SystemExit(
            f"ERROR: {label} has unsupported SquashFS version {major}.{minor}; expected 4.x"
        )
    rootfs_bytes_used = struct.unpack_from("<Q", blob, rootfs_offset + 40)[0]
    if rootfs_bytes_used < 96:
        raise SystemExit(
            f"ERROR: {label} SquashFS bytes_used is too small: "
            f"0x{rootfs_bytes_used:x}"
        )
    rootfs_checksum_offset = (rootfs_bytes_used + 0xFFF) & ~0xFFF
    checksum_start = rootfs_offset + rootfs_checksum_offset
    checksum_end = checksum_start + 2
    if checksum_end > len(blob):
        raise SystemExit(
            f"ERROR: {label} lacks the Realtek rootfs checksum: expected at "
            f"firmware+0x{checksum_start:x}, file size 0x{len(blob):x}"
        )
    if any(blob[rootfs_offset + rootfs_bytes_used:checksum_start]):
        raise SystemExit(
            f"ERROR: {label} has non-zero bytes in the rootfs checksum alignment gap"
        )
    rootfs_bootloader_marker = struct.unpack_from(">I", blob, rootfs_offset + 8)[0]
    expected_marker = (rootfs_checksum_offset + 2) - 0x282
    if rootfs_bootloader_marker != expected_marker:
        interpreted = rootfs_bootloader_marker + 0x282
        raise SystemExit(
            f"ERROR: {label} has invalid MW5 rootfs boot-length marker at "
            f"rootfs+0x08: 0x{rootfs_bootloader_marker:08x}, expected "
            f"0x{expected_marker:08x}; bootloader would check 0x{interpreted:x} "
            f"bytes instead of 0x{rootfs_checksum_offset + 2:x}"
        )

    rootfs_checksum_word = struct.unpack_from(">H", blob, checksum_start)[0]
    rootfs_checksum_sum = sum16_be(blob[rootfs_offset:checksum_end])
    if rootfs_checksum_sum != 0:
        raise SystemExit(
            f"ERROR: {label} Realtek rootfs checksum is invalid at "
            f"rootfs+0x{rootfs_checksum_offset:x}: word=0x{rootfs_checksum_word:04x}, "
            f"sum=0x{rootfs_checksum_sum:04x}"
        )

    return FirmwareInfo(
        signature=signature.decode("ascii"),
        load_addr=load_addr,
        burn_addr=burn_addr,
        image_len=image_len,
        rootfs_offset=rootfs_offset,
        checksum_sum=checksum_sum,
        rootfs_bytes_used=rootfs_bytes_used,
        rootfs_checksum_offset=rootfs_checksum_offset,
        rootfs_checksum_word=rootfs_checksum_word,
        rootfs_checksum_sum=rootfs_checksum_sum,
        rootfs_bootloader_marker=rootfs_bootloader_marker,
        rootfs_bootloader_check_length=rootfs_checksum_offset + 2,
    )


def region_hash(blob: bytes | bytearray, offset: int, size: int) -> str:
    return sha256(blob[offset:offset + size])


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--template", required=True, type=Path,
                    help="user's own complete 8 MiB MW5 SPI dump")
    ap.add_argument("--firmware", required=True, type=Path,
                    help="combined OpenWrt firmware-cr6c.bin")
    ap.add_argument("--output", required=True, type=Path,
                    help="output 8 MiB image for an external SPI programmer")
    ap.add_argument("--manifest", type=Path,
                    help="optional text manifest with hashes and preserved regions")
    args = ap.parse_args()

    if not args.template.is_file():
        ap.error(f"template does not exist: {args.template}")
    if not args.firmware.is_file():
        ap.error(f"firmware does not exist: {args.firmware}")

    template = bytearray(args.template.read_bytes())
    firmware = args.firmware.read_bytes()

    if len(template) != FLASH_SIZE:
        ap.error(
            f"template must be exactly 8 MiB / 0x{FLASH_SIZE:x} bytes; "
            f"got 0x{len(template):x}"
        )
    if len(firmware) > FIRMWARE_SIZE:
        ap.error(
            f"firmware is too large: 0x{len(firmware):x} > "
            f"0x{FIRMWARE_SIZE:x} bytes"
        )

    # Require a structurally valid MW5 stock/current firmware in the template.
    # This catches blank chips, truncated dumps and most foreign 8 MiB images.
    template_fw = bytes(template[FIRMWARE_OFFSET:FIRMWARE_OFFSET + FIRMWARE_SIZE])
    template_info = parse_firmware(template_fw, "template firmware partition")
    if not any(marker in template for marker in MW5_MODEL_MARKERS):
        raise SystemExit(
            "ERROR: template lacks MW5 model markers (sys.model=MW5/Mw5;); "
            "refusing a possible foreign 8 MiB Tenda dump"
        )
    firmware_info = parse_firmware(
        firmware, "OpenWrt combined firmware", require_immediate_payload=True
    )

    output = bytearray(template)
    fw_end = FIRMWARE_OFFSET + FIRMWARE_SIZE
    output[FIRMWARE_OFFSET:fw_end] = b"\xff" * FIRMWARE_SIZE
    output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)] = firmware

    if len(output) != FLASH_SIZE:
        raise SystemExit("ERROR: internal error: output size changed")
    if output[:FIRMWARE_OFFSET] != template[:FIRMWARE_OFFSET]:
        raise SystemExit("ERROR: bootloader/CFG area changed unexpectedly")
    if output[fw_end:] != template[fw_end:]:
        raise SystemExit("ERROR: CFM/LOG/ENV area changed unexpectedly")
    if output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)] != firmware:
        raise SystemExit("ERROR: inserted firmware differs from input")
    if any(b != 0xFF for b in output[FIRMWARE_OFFSET + len(firmware):fw_end]):
        raise SystemExit("ERROR: unused firmware tail is not erased to 0xff")

    # Parse the image again at its final flash offset.
    final_info = parse_firmware(
        bytes(output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)]),
        "final fullflash firmware",
        require_immediate_payload=True,
    )
    if final_info != firmware_info:
        raise SystemExit("ERROR: final firmware header differs after insertion")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    readback = args.output.read_bytes()
    if readback != output:
        raise SystemExit("ERROR: output readback differs from generated MW5 image")

    regions = [
        ("bootloader", BOOTLOADER_OFFSET, BOOTLOADER_SIZE),
        ("cfg", CFG_OFFSET, CFG_SIZE),
        ("firmware", FIRMWARE_OFFSET, FIRMWARE_SIZE),
        ("cfm", CFM_OFFSET, CFM_SIZE),
        ("cfm_backup", CFM_BACKUP_OFFSET, CFM_BACKUP_SIZE),
        ("log", LOG_OFFSET, LOG_SIZE),
        ("env", ENV_OFFSET, ENV_SIZE),
    ]

    lines = [
        "Tenda Nova MW5 personalized SPI fullflash manifest",
        "WARNING=contains private device data; do not publish",
        f"template={args.template}",
        f"template_size={len(template)}",
        f"template_sha256={sha256(template)}",
        f"template_signature={template_info.signature}",
        f"template_rootfs_offset=0x{template_info.rootfs_offset:x}",
        f"firmware={args.firmware}",
        f"firmware_size={len(firmware)}",
        f"firmware_sha256={sha256(firmware)}",
        f"firmware_signature={firmware_info.signature}",
        f"firmware_load_addr=0x{firmware_info.load_addr:08x}",
        f"firmware_burn_addr=0x{firmware_info.burn_addr:08x}",
        f"firmware_header_len=0x{firmware_info.image_len:x}",
        f"firmware_rootfs_offset=0x{firmware_info.rootfs_offset:x}",
        f"firmware_kernel_checksum_sum=0x{firmware_info.checksum_sum:04x}",
        f"firmware_rootfs_bytes_used=0x{firmware_info.rootfs_bytes_used:x}",
        f"firmware_rootfs_checksum_offset=0x{firmware_info.rootfs_checksum_offset:x}",
        f"firmware_rootfs_checksum_word=0x{firmware_info.rootfs_checksum_word:04x}",
        f"firmware_rootfs_checksum_sum=0x{firmware_info.rootfs_checksum_sum:04x}",
        f"firmware_rootfs_bootloader_marker=0x{firmware_info.rootfs_bootloader_marker:08x}",
        f"firmware_rootfs_bootloader_check_length=0x{firmware_info.rootfs_bootloader_check_length:x}",
        f"output={args.output}",
        f"output_size={len(output)}",
        f"output_sha256={sha256(output)}",
        "preserved_before_firmware=true",
        "preserved_after_firmware=true",
        "readback_verified=true",
    ]
    for name, offset, size in regions:
        lines.append(
            f"region_{name}=offset:0x{offset:x},size:0x{size:x},"
            f"sha256:{region_hash(output, offset, size)}"
        )

    if args.manifest:
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        args.manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print("rtl8197f-tenda-mw5-fullflash:")
    print(f"  template:        {args.template} sha256={sha256(template)}")
    print(f"  firmware:        {args.firmware} size=0x{len(firmware):x}")
    print(f"  signature:       {firmware_info.signature}")
    print(f"  load/burn:       0x{firmware_info.load_addr:08x}/0x{firmware_info.burn_addr:08x}")
    print(f"  rootfs offset:   firmware+0x{firmware_info.rootfs_offset:x}")
    print(f"  rootfs checksum: rootfs+0x{firmware_info.rootfs_checksum_offset:x} "
          f"word=0x{firmware_info.rootfs_checksum_word:04x} sum=0x{firmware_info.rootfs_checksum_sum:04x}")
    print(f"  rootfs marker:   0x{firmware_info.rootfs_bootloader_marker:08x} "
          f"-> check length 0x{firmware_info.rootfs_bootloader_check_length:x}")
    print(f"  replaced range:  0x{FIRMWARE_OFFSET:06x}..0x{fw_end:06x}")
    print(f"  output:          {args.output} size=0x{len(output):x}")
    print(f"  output sha256:   {sha256(output)}")
    print("  preserved:       bootloader, CFG, CFM, CFM_BACKUP, LOG and ENV")
    if args.manifest:
        print(f"  manifest:        {args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
