#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Verify archived OEM kernels and every word of their full disassembly.

Requires only Python's standard library, not an SPI dump or cross toolchain.
"""
import argparse
import hashlib
import json
import lzma
from pathlib import Path
import re
import struct


WORD = re.compile(rb'^([0-9a-f]{8}): ([0-9a-f]{8})  ', re.MULTILINE)
LIMIT = 64 * 1024 * 1024


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify(folder):
    folder = folder.resolve()
    manifest = json.loads((folder / 'manifest.json').read_text())

    def read(name):
        path = (folder / name).resolve()
        if not path.is_relative_to(folder):
            raise ValueError('artifact path escapes archive')
        expected = manifest['artifacts'][name]
        data = path.read_bytes()
        if len(data) != expected['size'] or sha(data) != expected['sha256']:
            raise ValueError('artifact size/hash mismatch: ' + name)
        if name.endswith('.xz'):
            decoder = lzma.LZMADecompressor()
            data = decoder.decompress(data, max_length=LIMIT + 1)
            if not decoder.eof or decoder.unused_data or len(data) > LIMIT:
                raise ValueError('invalid or oversized XZ artifact: ' + name)
        if (len(data) != expected['unpacked_size'] or
                sha(data) != expected['unpacked_sha256']):
            raise ValueError('unpacked artifact size/hash mismatch: ' + name)
        return data

    kernel = read('kernel.raw.xz')
    if len(kernel) != manifest['kernel_size'] or sha(kernel) != manifest['kernel_sha256']:
        raise ValueError('kernel identity mismatch')
    symbols = read('symbols.tsv.xz').splitlines()
    if len(symbols) - 1 != manifest['symbol_count']:
        raise ValueError('symbol count mismatch')
    base = manifest['image_base']
    for line in read('text-symbol-spans.tsv.xz').splitlines()[1:]:
        start, stop, size, digest, names = line.split(b'\t', 4)
        start, stop = int(start, 16) - base, int(stop, 16) - base
        if not (0 <= start < stop <= len(kernel) and stop - start == int(size)):
            raise ValueError('invalid text-symbol range')
        if sha(kernel[start:stop]).encode() != digest:
            raise ValueError('text-symbol span hash mismatch')
    expected = base
    checked = {'kernel.raw.xz', 'symbols.tsv.xz', 'text-symbol-spans.tsv.xz'}
    for chunk in manifest['disassembly_chunks']:
        if chunk['start'] != expected or chunk['stop'] <= expected:
            raise ValueError('disassembly chunk gap or overlap')
        count = 0
        for match in WORD.finditer(read(chunk['path'])):
            address, word = (int(value, 16) for value in match.groups())
            if address != expected or address + 4 > base + len(kernel):
                raise ValueError('disassembly instruction gap, overlap or out-of-range word')
            if word != struct.unpack_from('<I', kernel, address - base)[0]:
                raise ValueError('disassembly word differs from original kernel')
            expected += 4
            count += 1
        if expected != chunk['stop'] or count != chunk['words']:
            raise ValueError('disassembly chunk coverage mismatch')
        checked.add(chunk['path'])
    if expected != base + len(kernel):
        raise ValueError('disassembly does not cover the complete kernel')
    for name in manifest['artifacts'].keys() - checked:
        read(name)
    return manifest


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('folders', nargs='+', type=Path)
    args = ap.parse_args()
    try:
        for folder in args.folders:
            manifest = verify(folder)
            print('%s: verified all %d kernel bytes, %d symbols and complete disassembly' %
                  (manifest['board'], manifest['kernel_size'], manifest['symbol_count']))
    except (OSError, ValueError, KeyError, lzma.LZMAError) as exc:
        ap.exit(1, str(exc) + '\n')


if __name__ == '__main__':
    main()
