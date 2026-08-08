#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build and verify a personalized Tenda AC23/Lynx 8 MiB SPI image.

The OpenWrt input is one combined cr6c firmware image.  The complete verified
firmware window 0x030000..0x7e0000 is erased and replaced.  Bootloader,
factory/RF calibration and the two top-of-flash Tenda configuration copies are
preserved byte-for-byte from the user's own SPI dump.

The stock AC23 dump contains a valid Realtek kernel checksum but no plain
SquashFS magic.  Generated OpenWrt firmware is intentionally stricter: its
SquashFS location is derived from IMG_HEADER_T.len and both the kernel and
Tenda rootfs checksums are validated before and after insertion.
"""
from __future__ import annotations
import argparse
import hashlib
import struct
from dataclasses import dataclass
from pathlib import Path

FLASH_SIZE = 0x800000
BOOT_FACTORY_END = 0x030000
FIRMWARE_OFFSET = 0x030000
FIRMWARE_END = 0x7E0000
FIRMWARE_SIZE = FIRMWARE_END - FIRMWARE_OFFSET
CONFIG_OFFSET = 0x7E0000
CONFIG_SIZE = 0x020000
LOGICAL_HEADER_SIZE = 16
EXPECTED_HEADER_SIZE = 60
EXPECTED_LOADADDR = 0x80A00000
EXPECTED_BURNADDR = FIRMWARE_OFFSET
MODEL_MARKERS = (b"TENDA.", b"Lynx_")

@dataclass(frozen=True)
class HeaderInfo:
    signature: str
    load_addr: int
    burn_addr: int
    image_len: int
    kernel_end: int
    checksum_sum: int

@dataclass(frozen=True)
class FirmwareInfo:
    header: HeaderInfo
    rootfs_offset: int
    rootfs_bytes_used: int
    rootfs_checksum_offset: int
    rootfs_checksum_word: int
    rootfs_checksum_sum: int

def sha256(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()

def sum16_be(data: bytes) -> int:
    total = 0
    for i in range(0, len(data) & ~1, 2):
        total = (total + ((data[i] << 8) | data[i + 1])) & 0xFFFF
    if len(data) & 1:
        total = (total + (data[-1] << 8)) & 0xFFFF
    return total

def parse_header(blob: bytes, label: str) -> HeaderInfo:
    if len(blob) < LOGICAL_HEADER_SIZE:
        raise SystemExit(f"ERROR: {label} is shorter than IMG_HEADER_T")
    signature = blob[:4]
    if signature != b"cr6c":
        raise SystemExit(f"ERROR: {label} signature is {signature!r}, expected b'cr6c'")
    load_addr, burn_addr, image_len = struct.unpack_from(">III", blob, 4)
    if load_addr != EXPECTED_LOADADDR:
        raise SystemExit(f"ERROR: {label} load address 0x{load_addr:08x} != 0x{EXPECTED_LOADADDR:08x}")
    if burn_addr != EXPECTED_BURNADDR:
        raise SystemExit(f"ERROR: {label} burn address 0x{burn_addr:08x} != 0x{EXPECTED_BURNADDR:08x}")
    kernel_end = LOGICAL_HEADER_SIZE + image_len
    if image_len < EXPECTED_HEADER_SIZE - LOGICAL_HEADER_SIZE + 2:
        raise SystemExit(f"ERROR: {label} IMG_HEADER_T.len is implausibly small: 0x{image_len:x}")
    if kernel_end > len(blob):
        raise SystemExit(f"ERROR: {label} kernel checksum range ends outside file at 0x{kernel_end:x}")
    checksum_sum = sum16_be(blob[LOGICAL_HEADER_SIZE:kernel_end])
    if checksum_sum:
        raise SystemExit(f"ERROR: {label} kernel checksum sum is 0x{checksum_sum:04x}, expected 0")
    return HeaderInfo(signature.decode(), load_addr, burn_addr, image_len, kernel_end, checksum_sum)

def parse_openwrt_firmware(blob: bytes, label: str) -> FirmwareInfo:
    header = parse_header(blob, label)
    rootfs_offset = header.kernel_end
    if rootfs_offset + 96 > len(blob) or blob[rootfs_offset:rootfs_offset + 4] != b"hsqs":
        raise SystemExit(f"ERROR: {label} has no SquashFS at IMG_HEADER_T-derived offset 0x{rootfs_offset:x}")
    major, minor = struct.unpack_from("<HH", blob, rootfs_offset + 28)
    if major != 4:
        raise SystemExit(f"ERROR: {label} SquashFS version is {major}.{minor}, expected 4.x")
    rootfs_bytes_used = struct.unpack_from("<Q", blob, rootfs_offset + 40)[0]
    if rootfs_bytes_used < 96:
        raise SystemExit(f"ERROR: {label} SquashFS bytes_used is too small: 0x{rootfs_bytes_used:x}")
    checksum_offset = (rootfs_bytes_used + 0xFFF) & ~0xFFF
    checksum_start = rootfs_offset + checksum_offset
    checksum_end = checksum_start + 2
    if checksum_end > len(blob):
        raise SystemExit(f"ERROR: {label} rootfs checksum lies outside firmware at 0x{checksum_start:x}")
    gap = blob[rootfs_offset + rootfs_bytes_used:checksum_start]
    if any(gap):
        raise SystemExit(f"ERROR: {label} rootfs checksum alignment gap is not zero-filled")
    checksum_word = struct.unpack_from(">H", blob, checksum_start)[0]
    checksum_sum = sum16_be(blob[rootfs_offset:checksum_end])
    if checksum_sum:
        raise SystemExit(f"ERROR: {label} rootfs checksum word 0x{checksum_word:04x} yields sum 0x{checksum_sum:04x}")
    return FirmwareInfo(header, rootfs_offset, rootfs_bytes_used, checksum_offset, checksum_word, checksum_sum)

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--template", required=True, type=Path, help="user's own complete 8 MiB AC23/Lynx SPI dump")
    ap.add_argument("--firmware", required=True, type=Path, help="combined OpenWrt firmware-cr6c.bin")
    ap.add_argument("--output", required=True, type=Path)
    ap.add_argument("--manifest", type=Path)
    args = ap.parse_args()

    template = args.template.read_bytes()
    firmware = args.firmware.read_bytes()
    if len(template) != FLASH_SIZE:
        ap.error(f"template must be 0x{FLASH_SIZE:x} bytes, got 0x{len(template):x}")
    if len(firmware) > FIRMWARE_SIZE:
        ap.error(f"firmware is 0x{len(firmware):x}, maximum is 0x{FIRMWARE_SIZE:x}")
    if not all(marker in template for marker in MODEL_MARKERS):
        raise SystemExit("ERROR: template lacks TENDA. and Lynx_ factory markers")

    template_fw = template[FIRMWARE_OFFSET:FIRMWARE_END]
    template_header = parse_header(template_fw, "stock AC23 template firmware")
    # The stock MIPS loader starts at byte 0x3c.  Detecting a different padding
    # pattern prevents silently using a foreign Tenda cr6c layout.
    if any(template_fw[LOGICAL_HEADER_SIZE:EXPECTED_HEADER_SIZE]) or template_fw[EXPECTED_HEADER_SIZE] == 0:
        raise SystemExit("ERROR: template does not match the observed AC23 60-byte cr6c header layout")
    firmware_info = parse_openwrt_firmware(firmware, "OpenWrt AC23 combined firmware")

    output = bytearray(template)
    output[FIRMWARE_OFFSET:FIRMWARE_END] = b"\xff" * FIRMWARE_SIZE
    output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)] = firmware
    if output[:BOOT_FACTORY_END] != template[:BOOT_FACTORY_END]:
        raise SystemExit("ERROR: bootloader/factory area changed")
    if output[CONFIG_OFFSET:] != template[CONFIG_OFFSET:]:
        raise SystemExit("ERROR: Tenda configuration copies changed")
    unused_tail = output[FIRMWARE_OFFSET + len(firmware):FIRMWARE_END]
    if any(byte != 0xFF for byte in unused_tail):
        raise SystemExit("ERROR: unused firmware tail is not erased")

    final_info = parse_openwrt_firmware(bytes(output[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware)]), "final AC23 fullflash firmware")
    if final_info != firmware_info:
        raise SystemExit("ERROR: firmware metadata changed during insertion")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    readback = args.output.read_bytes()
    if readback != output:
        raise SystemExit("ERROR: output readback differs from generated image")

    lines = [
        "Tenda AC23/Lynx personalized SPI fullflash manifest v42",
        "WARNING=contains private device data; do not publish",
        f"template={args.template}",
        f"template_size=0x{len(template):x}",
        f"template_sha256={sha256(template)}",
        f"template_header_len=0x{template_header.image_len:x}",
        f"template_kernel_checksum_sum=0x{template_header.checksum_sum:04x}",
        f"firmware={args.firmware}",
        f"firmware_size=0x{len(firmware):x}",
        f"firmware_sha256={sha256(firmware)}",
        f"firmware_header_size=0x{EXPECTED_HEADER_SIZE:x}",
        f"firmware_header_len=0x{firmware_info.header.image_len:x}",
        f"firmware_rootfs_offset=0x{firmware_info.rootfs_offset:x}",
        f"firmware_rootfs_bytes_used=0x{firmware_info.rootfs_bytes_used:x}",
        f"firmware_rootfs_checksum_offset=0x{firmware_info.rootfs_checksum_offset:x}",
        f"firmware_rootfs_checksum_word=0x{firmware_info.rootfs_checksum_word:04x}",
        f"output={args.output}",
        f"output_size=0x{len(output):x}",
        f"output_sha256={sha256(output)}",
        f"preserved_boot_factory_sha256={sha256(output[:BOOT_FACTORY_END])}",
        f"preserved_tenda_config_sha256={sha256(output[CONFIG_OFFSET:])}",
        "readback_verified=true",
    ]
    if args.manifest:
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        args.manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print("rtl8197f-tenda-lynx-fullflash v42:")
    print(f"  firmware window: 0x{FIRMWARE_OFFSET:06x}..0x{FIRMWARE_END:06x}")
    print(f"  cr6c header:      {EXPECTED_HEADER_SIZE} bytes, checksum OK")
    print(f"  rootfs:           firmware+0x{firmware_info.rootfs_offset:x}, checksum OK")
    print(f"  preserved:        boot/factory 0x0..0x30000, config 0x7e0000..0x800000")
    print(f"  output sha256:    {sha256(output)}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
