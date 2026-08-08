#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build a Realtek RTL8197F/RD05 bootmiwifi-style image header.

The Realtek boot code used by RTL8197-class platforms recognises a small
firmware header before the Linux payload.  The vendor BSP names this structure
IMG_HEADER_T and checks signatures such as csys/cs6c before copying
``len - 2`` bytes from flash to ``startAddr`` and jumping there.

This tool intentionally implements only the public, non-cryptographic header
variant:

    signature[4] + startAddr[be32] + burnAddr[be32] + len[be32]
    + payload + optional zero pad + checksum[be16] + optional ff padding

The checksum is the Realtek 16-bit additive checksum used by cvimg/bootcode: the
16-bit big-endian sum over payload+checksum must be zero.  Xiaomi RD05 stock
images use the RTL8198/RTL8197F-style ``cs6c`` signature at flash offset
0x60000 with startAddr 0x80cf0000.  Secure-signature variants are deliberately not generated here.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


def parse_u32(value: str) -> int:
    try:
        out = int(value, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    if not 0 <= out <= 0xFFFFFFFF:
        raise argparse.ArgumentTypeError(f"value out of u32 range: {value}")
    return out


def checksum16_be(data: bytes) -> int:
    """Return checksum word making BE 16-bit sum(payload + csum) == 0."""
    total = 0
    for i in range(0, len(data) & ~1, 2):
        total = (total + ((data[i] << 8) | data[i + 1])) & 0xFFFF
    if len(data) & 1:
        total = (total + (data[-1] << 8)) & 0xFFFF
    return (-total) & 0xFFFF


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("input", type=Path)
    ap.add_argument("output", type=Path)
    ap.add_argument("--signature", default="cs6c", help="4-byte image signature, default: cs6c")
    ap.add_argument("--start-addr", type=parse_u32, default=0x80CF0000,
                    help="RAM load/jump address stored in header; RD05 stock uses 0x80cf0000")
    ap.add_argument("--burn-addr", type=parse_u32, default=0x00060000,
                    help="flash burn address stored in header")
    ap.add_argument("--header-size", type=parse_u32, default=16,
                    help="header area size, minimum 16; extra bytes are zero padding")
    ap.add_argument("--pad-to", type=parse_u32, default=0,
                    help="pad final image to this byte multiple")
    ap.add_argument("--little-endian-header", action="store_true",
                    help="write header integers little-endian; default is big-endian as used by RD05 SPI dump")
    args = ap.parse_args(argv)

    sig = args.signature.encode("ascii", errors="strict")
    if len(sig) != 4:
        ap.error("--signature must be exactly 4 ASCII bytes")
    if args.header_size < 16:
        ap.error("--header-size must be at least 16")

    payload = args.input.read_bytes()

    # Realtek cvimg pads odd-sized payloads to an even length before appending
    # the 16-bit checksum word.  Without this pad byte, images with an odd
    # payload length produce a header len that is itself odd and the bootloader
    # checksum over header+0x10..header+0x10+len does not fold to zero.
    payload_pad = b"\x00" if (len(payload) & 1) else b""

    if len(payload) + len(payload_pad) + 2 > 0xFFFFFFFF:
        ap.error("payload too large")

    # The Realtek boot code copies len-2 bytes and expects the checksum word at
    # the end of the header-described payload region.  Therefore len includes
    # the checksum word and any odd-length pad byte, but excludes IMG_HEADER_T.
    length = len(payload) + len(payload_pad) + 2
    endian = "<" if args.little_endian_header else ">"
    header = sig + struct.pack(f"{endian}III", args.start_addr, args.burn_addr, length)
    header += b"\x00" * (args.header_size - 16)

    csum = checksum16_be(payload + payload_pad)
    out = header + payload + payload_pad + struct.pack(">H", csum)
    if args.pad_to:
        rem = len(out) % args.pad_to
        if rem:
            out += b"\xff" * (args.pad_to - rem)

    args.output.write_bytes(out)
    print(
        f"rtl8197f-image: sig={args.signature} start=0x{args.start_addr:08x} "
        f"burn=0x{args.burn_addr:08x} len=0x{length:08x} "
        f"checksum=0x{csum:04x} out={args.output}",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
