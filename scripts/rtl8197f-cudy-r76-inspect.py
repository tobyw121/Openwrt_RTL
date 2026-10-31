#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Inspect a Cudy R76 raw SPI dump without executing or exporting OEM userspace."""
import argparse
import hashlib
import json
import lzma
from pathlib import Path
import struct
import zlib


FLASH_SIZE = 0x1000000
KERNEL_OFFSET = 0x40000
ROOTFS_OFFSET = 0x2b0000
PARTITIONS = [('u-boot', 0, 0x20000), ('cfg', 0x20000, 0x10000),
              ('bdinfo', 0x30000, 0x10000), ('firmware', 0x40000, 0xec0000),
              ('kernel', 0x40000, 0x270000), ('rootfs', 0x2b0000, 0xc50000),
              ('rootfs_data', 0xf00000, 0xf0000), ('backup', 0xff0000, 0x10000)]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def kernel_string(kernel, prefix):
    pos = kernel.find(prefix)
    if pos < 0:
        return None
    return kernel[pos:pos+256].split(b'\0', 1)[0].decode('ascii', errors='replace').rstrip()


def partition_table(kernel):
    # OEM 32-bit mtd_partition: pointer, padding, u64 size, u64 offset, flags.
    candidates = []
    for start in range(0, len(kernel) - 8 * 32 + 1, 4):
        pointer, pad, size, high, off, off_high, _, _ = struct.unpack_from('<8I', kernel, start)
        if (pad, size, high, off, off_high) != (0, 0x20000, 0, 0, 0):
            continue
        if not 0x80000000 <= pointer < 0x80000000 + len(kernel):
            continue
        records = []
        for i, (name, expected_offset, expected_size) in enumerate(PARTITIONS):
            record = start + i * 32
            pointer, pad, size, high, off, off_high, _, _ = struct.unpack_from('<8I', kernel, record)
            pos = pointer - 0x80000000
            if not 0 <= pos < len(kernel):
                break
            if kernel[pos:pos+len(name)+1] != name.encode() + b'\0':
                break
            if pad or high or off_high or (off, size) != (expected_offset, expected_size):
                break
            records.append({'name': name, 'offset': off, 'size': size,
                            'kernel_record_offset': record})
        if len(records) == len(PARTITIONS):
            candidates.append(records)
    if len(candidates) != 1:
        raise ValueError('no unique compiled R76 partition table matching the target layout')
    return candidates[0]


def inspect(data):
    if len(data) != FLASH_SIZE:
        raise ValueError('expected a complete 16 MiB SPI dump, not an OEM upgrade file')
    header = data[KERNEL_OFFSET:KERNEL_OFFSET+64]
    magic, hcrc, timestamp, length, load, entry, dcrc, os_id, arch, kind, comp, name = struct.unpack('>7I4B32s', header)
    if magic != 0x27051956 or name.rstrip(b'\0') != b'R76':
        raise ValueError('missing R76 uImage at physical offset 0x40000')
    check = bytearray(header)
    check[4:8] = bytes(4)
    if zlib.crc32(check) != hcrc or length > ROOTFS_OFFSET - KERNEL_OFFSET - 64:
        raise ValueError('invalid uImage header checksum or kernel boundary')
    payload = data[KERNEL_OFFSET+64:KERNEL_OFFSET+64+length]
    if zlib.crc32(payload) != dcrc:
        raise ValueError('invalid uImage payload checksum')
    if payload[:4] != b'cr6c':
        raise ValueError('missing inner Realtek cr6c header')
    ram_entry, burn, inner_length = struct.unpack_from('>3I', payload, 4)
    if ram_entry != entry or burn != KERNEL_OFFSET or inner_length + 16 != length:
        raise ValueError('inconsistent Realtek/uImage sizes or addresses')
    # OEM rtkload stream location, checked within the CRC-verified payload.
    stream_offset = 0x2818
    decoder = lzma.LZMADecompressor(format=lzma.FORMAT_ALONE)
    kernel = decoder.decompress(payload[stream_offset:], max_length=32*1024*1024)
    if not decoder.eof:
        raise ValueError('incomplete or oversized kernel LZMA stream')
    if data[ROOTFS_OFFSET:ROOTFS_OFFSET+4] != b'hsqs':
        raise ValueError('missing SquashFS at physical offset 0x2b0000')
    inodes, _, block_size, _ = struct.unpack_from('<4I', data, ROOTFS_OFFSET+4)
    compressor = struct.unpack_from('<H', data, ROOTFS_OFFSET+20)[0]
    major, minor = struct.unpack_from('<2H', data, ROOTFS_OFFSET+28)
    used = struct.unpack_from('<Q', data, ROOTFS_OFFSET+40)[0]
    if (major, minor, compressor) != (4, 0, 4) or not 96 <= used <= 0xc50000:
        raise ValueError('unexpected or oversized SquashFS filesystem')
    report = {
        'schema': 1, 'board': 'cudy,ax1500-r76', 'oem_profile': 'R76',
        'source_size': len(data), 'source_sha256': sha(data),
        'classification': 'complete raw SPI layout; includes per-device cfg/bdinfo',
        'uimage': {'offset': KERNEL_OFFSET, 'payload_size': length,
                   'header_crc32_valid': True, 'data_crc32_valid': True,
                   'timestamp': timestamp, 'load_field': load, 'entry': entry,
                   'os': os_id, 'arch': arch, 'type': kind, 'compression': comp},
        'realtek_header': {'signature': 'cr6c', 'burn_address': burn, 'ram_entry': ram_entry},
        'kernel': {'size': len(kernel), 'sha256': sha(kernel),
                   'lzma_spi_offset': KERNEL_OFFSET+64+stream_offset,
                   'version_string': kernel_string(kernel, b'Linux version '),
                   'compiled_console': kernel_string(kernel, b'console=ttyS0,')},
        'squashfs': {'offset': ROOTFS_OFFSET, 'bytes_used': used,
                     'inodes': inodes, 'block_size': block_size, 'compression': 'XZ'},
        'partitions': partition_table(kernel),
        'cfg_and_bdinfo_nonblank': all(data[o:o+s] != b'\xff'*s and data[o:o+s] != bytes(s)
                                     for _, o, s in PARTITIONS[1:3]),
        'limits': ['No hardware revision, physical RAM capacity, live switch/PCI ID or read-shift measurement.',
                   'Header CRCs verify consistency, not OEM authenticity.',
                   'No executable was run; cfg/bdinfo and OEM userspace are not exported.']}
    return report, kernel


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('dump', type=Path)
    p.add_argument('--report', required=True, type=Path)
    p.add_argument('--kernel-output', type=Path, help='optional local kernel extraction')
    args = p.parse_args()
    try:
        report, kernel = inspect(args.dump.read_bytes())
    except (ValueError, OSError, lzma.LZMAError) as exc:
        p.error(str(exc))
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + '\n')
    if args.kernel_output:
        args.kernel_output.write_bytes(kernel)
    print('R76 SPI layout and checksums verified; report:', args.report)


if __name__ == '__main__':
    main()
