#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build a private Cudy R76 16 MiB SPI image; never writes a router."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import zlib

KERNEL_OFF = 0x40000
ROOTFS_OFF = 0x440000
END = 0xf00000
ENTRY = 0x82000000


def digest(data):
    return hashlib.sha256(data).hexdigest()


def wrap(loader):
    if not loader:
        raise ValueError('empty loader/kernel')
    payload = loader + (b'\0' if len(loader) % 2 else b'')
    checksum = (-sum(struct.unpack('>'+str(len(payload)//2)+'H', payload))) & 65535
    inner = b'cr6c' + struct.pack('>III', ENTRY, KERNEL_OFF, len(payload)+2)
    inner += payload + struct.pack('>H', checksum)
    header = struct.pack('>7I4B32s', 0x27051956, 0, 0, len(inner),
                         KERNEL_OFF, ENTRY, zlib.crc32(inner), 5, 5, 2, 3, b'R76')
    header = header[:4] + struct.pack('>I', zlib.crc32(header)) + header[8:]
    return header + inner


def build(template, loader, rootfs):
    spec = importlib.util.spec_from_file_location('r76_inspect', Path(__file__).with_name('rtl8197f-cudy-r76-inspect.py'))
    inspector = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inspector)
    inspector.inspect(template)  # Stock format, both CRCs and compiled partition map.
    kernel = wrap(loader)
    if len(kernel) > ROOTFS_OFF-KERNEL_OFF:
        raise ValueError('kernel exceeds 4 MiB Cudy kernel partition')
    if len(rootfs) < 96 or rootfs[:4] != b'hsqs':
        raise ValueError('expected complete SquashFS rootfs')
    used = struct.unpack_from('<Q', rootfs, 40)[0]
    if struct.unpack_from('<HH', rootfs, 28) != (4, 0) or not 96 <= used <= len(rootfs):
        raise ValueError('invalid SquashFS superblock/length')
    if len(rootfs) > END-ROOTFS_OFF:
        raise ValueError('rootfs exceeds Cudy rootfs partition')
    out = bytearray(template)
    out[KERNEL_OFF:END] = b'\xff' * (END-KERNEL_OFF)
    out[KERNEL_OFF:KERNEL_OFF+len(kernel)] = kernel
    out[ROOTFS_OFF:ROOTFS_OFF+len(rootfs)] = rootfs
    assert len(out) == 0x1000000
    assert out[:KERNEL_OFF] == template[:KERNEL_OFF] and out[END:] == template[END:]
    return bytes(out), {'template_sha256': digest(template), 'output_sha256': digest(out),
        'kernel_offset': KERNEL_OFF, 'rootfs_offset': ROOTFS_OFF,
        'kernel_size': len(kernel), 'rootfs_size': len(rootfs), 'entry': ENTRY,
        'preserved_prefix_sha256': digest(out[:KERNEL_OFF]),
        'preserved_tail_sha256': digest(out[END:]),
        'warning': 'Private device-specific SPI image. Experimental; no hardware boot verified.'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('template', 'kernel', 'rootfs', 'output'):
        p.add_argument('--'+name, required=True, type=Path)
    p.add_argument('--manifest', required=True, type=Path)
    args = p.parse_args()
    try:
        output, manifest = build(args.template.read_bytes(), args.kernel.read_bytes(), args.rootfs.read_bytes())
        if args.output.resolve() in (args.template.resolve(), args.kernel.resolve(), args.rootfs.resolve()):
            raise ValueError('output must not overwrite an input')
        if args.manifest.resolve() in (args.template.resolve(), args.kernel.resolve(), args.rootfs.resolve(), args.output.resolve()):
            raise ValueError('manifest must be a separate file')
    except (OSError, ValueError) as exc:
        p.error(str(exc))
    args.output.write_bytes(output)
    args.manifest.write_text(json.dumps(manifest, indent=2)+'\n')
    print('Cudy R76 fullflash: 16 MiB; boot/cfg/bdinfo and OEM tail preserved')


if __name__ == '__main__':
    main()
