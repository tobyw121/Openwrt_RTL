#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Archive and verify a complete kernel from one of the three known SPI dumps.

Never archives the SPI image, rootfs, environment or factory partitions.
Disassembly covers every image word, including data interpreted as instructions;
it is not recovered C source or proof that every word is executable code.
"""
import argparse
import hashlib
import importlib.util
import json
import lzma
from pathlib import Path
import re
import struct
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SOURCES = {
    'mw5': ('tenda-nova-mw5-spi.bin', 0x32818,
            'b4b1b2caca98e20096a80519b9c8ff22399d051bc77f6aef4f29b6ef76a31428'),
    'ac23': ('tenda-ac23-spi.bin', 0x32818,
             '300c7216dbfa1d7d930a8db896ec755edb0c41bd3c1b3ab90e4befcd045aebf5'),
    'rd05': ('xiaomi-r4-rd05-spi.bin', 0x62818,
             '805469c49ecd6860a444f92ef19bcf4834ee912261b4a0205074d6095c516608'),
}
CHUNK_SIZE = 1024 * 1024
LINE = re.compile(r'^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)$')
JUMP = re.compile(r'\b(?:jal|j)\s+0x([0-9a-f]+)')


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unpack_kernel(flash, offset):
    decoder = lzma.LZMADecompressor(format=lzma.FORMAT_ALONE)
    kernel = decoder.decompress(flash[offset:], max_length=64 * 1024 * 1024 + 1)
    if not decoder.eof or len(kernel) > 64 * 1024 * 1024 or len(kernel) % 4:
        raise ValueError('incomplete, oversized or unaligned kernel stream')
    return kernel, len(flash) - offset - len(decoder.unused_data)


def privacy_check(flash, kernel, board):
    windows = [(0x20000, 0x30000)] if board != 'rd05' else [
        (0x20000, 0x30000), (0x30000, 0x40000)]
    patterns = set()
    for start, stop in windows:
        for match in re.finditer(rb'([\w.]+)=([^\x00\xff\r\n]+)', flash[start:stop]):
            key, value = match.groups()
            if re.search(rb'mac|ethaddr|ssid|pass|pwd|secret|psk|serial', key, re.I):
                if len(value) >= 8:
                    patterns.add(value)
                clean = re.sub(rb'[^0-9a-fA-F]', b'', value)
                if re.search(rb'mac|ethaddr', key, re.I) and len(clean) == 12:
                    mac = bytes.fromhex(clean.decode())
                    if mac not in (b'\0' * 6, b'\xff' * 6):
                        patterns.add(mac)
    if any(p in kernel for p in patterns) or re.search(
            rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----', kernel):
        raise ValueError('kernel privacy check requires manual review; not archived')
    return {'identifier_patterns_checked': len(patterns), 'matches': 0,
            'private_key_markers': 0,
            'limits': 'Known env/factory ASCII identifiers, binary MAC forms and PEM '
                      'markers only; not a general guarantee that compiled OEM '
                      'defaults contain no sensitive strings.'}


def annotate_disassembly(output, blob, start, stop, base, labels):
    """Verify every objdump word against the image and reject omissions."""
    expected = start
    lines = []
    for line in output.splitlines():
        match = LINE.match(line)
        if not match:
            continue
        address, word = int(match[1], 16), int(match[2], 16)
        if address != expected or address >= stop:
            raise ValueError('non-contiguous disassembly at 0x%x' % expected)
        if word != struct.unpack_from('<I', blob, address - base)[0]:
            raise ValueError('disassembly/image word mismatch at 0x%x' % address)
        for symbol in labels.get(address, []):
            lines.append('%08x <%s>:' % (address, symbol))
        instruction = match[3]
        jump = JUMP.search(instruction)
        if jump and int(jump[1], 16) in labels:
            instruction += ' # ' + ', '.join(labels[int(jump[1], 16)])
        lines.append('%08x: %08x  %s' % (address, word, instruction))
        expected += 4
    if expected != stop:
        raise ValueError('disassembly ends before 0x%x' % stop)
    return '\n'.join(lines) + '\n'


def archive(args):
    filename, offset, expected_flash = SOURCES[args.board]
    flash = args.dump.read_bytes()
    if sha(flash) != expected_flash:
        raise ValueError('input is not the recorded reference SPI image')
    blob, compressed_size = unpack_kernel(flash, offset)
    evidence = json.loads((ROOT / 'target/linux/realtek/rtl8197f/oem-switch-evidence.json').read_text())
    if sha(blob) != evidence['boards'][args.board]['kernel_sha256']:
        raise ValueError('kernel does not match recorded evidence')
    privacy = privacy_check(flash, blob, args.board)
    spec = importlib.util.spec_from_file_location(
        'kernel_kallsyms', Path(__file__).with_name('rtl8197f-oem-kallsyms.py'))
    decoder = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(decoder)
    recovered = decoder.recover(blob)
    base, symbols = recovered['image_base'], recovered['symbols']
    labels = {}
    typed = {}
    for symbol in symbols:
        labels.setdefault(symbol['address'], []).append(symbol['name'])
        typed.setdefault(symbol['address'], []).append(symbol)
    addresses = sorted(labels)
    manifest = {'schema': 1, 'board': args.board,
                'source_commit': evidence['source_commit'],
                'source_path': evidence['dump_directory'] + '/' + filename,
                'flash_sha256': expected_flash, 'lzma_offset': offset,
                'lzma_size': compressed_size, 'image_base': base,
                'kernel_size': len(blob), 'kernel_sha256': sha(blob),
                'symbol_count': recovered['symbol_count'],
                'export_cross_checks': recovered['export_cross_checks'],
                'kallsyms_offsets': recovered['offsets'], 'privacy_checks': privacy,
                'objdump': subprocess.check_output([args.objdump, '--version'], text=True).splitlines()[0],
                'limits': 'Full loaded kernel image; no rootfs/modules, BSS contents, '
                          'original C source or complete semantic reverse engineering. '
                          'Whole-image disassembly also interprets data as code.',
                'artifacts': {}, 'disassembly_chunks': []}
    # A fresh output directory prevents silent overwrite of previous analyses.
    args.output.mkdir(parents=True, exist_ok=False)

    def save(name, data, compress=True):
        if isinstance(data, str):
            data = data.encode()
        raw_hash, raw_size = sha(data), len(data)
        content = lzma.compress(data, preset=6) if compress else data
        path = args.output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
        manifest['artifacts'][name] = {'sha256': sha(content), 'size': len(content),
                                       'unpacked_sha256': raw_hash, 'unpacked_size': raw_size}

    save('kernel.raw.xz', blob)
    save('symbols.tsv.xz', 'address\ttype\tname\n' + ''.join(
        '%08x\t%s\t%s\n' % (s['address'], s['type'], s['name']) for s in symbols))
    spans = ['start\tstop\tsize\tsha256\ttext_symbols\n']
    for i, address in enumerate(addresses):
        names = [s['name'] for s in typed[address]
                 if s['type'] in 'Tt']
        stop = addresses[i + 1] if i + 1 < len(addresses) else base + len(blob)
        if names and base <= address < stop <= base + len(blob):
            code = blob[address - base:stop - base]
            spans.append('%08x\t%08x\t%d\t%s\t%s\n' %
                         (address, stop, len(code), sha(code), ','.join(names)))
    save('text-symbol-spans.tsv.xz', ''.join(spans))
    markers = {s['name']: s['address'] for s in symbols if s['name'] in {
        '_text', '_stext', '_etext', '_sdata', '_edata', '__start_rodata',
        '__end_rodata', '__init_begin', '__init_end', '_sinittext', '_einittext',
        '__bss_start', '__bss_stop', '_end'}}
    save('linker-markers.json', json.dumps(markers, indent=2) + '\n', False)
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / 'kernel.raw'
        raw.write_bytes(blob)
        for pos in range(0, len(blob), CHUNK_SIZE):
            start, stop = base + pos, base + min(pos + CHUNK_SIZE, len(blob))
            command = [args.objdump, '-D', '-z', '-b', 'binary', '-m', 'mips:isa32r2', '-EL',
                       '--adjust-vma=0x%x' % base, '--start-address=0x%x' % start,
                       '--stop-address=0x%x' % stop, str(raw)]
            output = subprocess.check_output(command, text=True)
            text = annotate_disassembly(output, blob, start, stop, base, labels)
            name = 'disassembly/%08x-%08x.dis.xz' % (start, stop)
            save(name, text)
            manifest['disassembly_chunks'].append({'path': name, 'start': start,
                                                   'stop': stop, 'words': (stop - start) // 4})
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    return manifest


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--board', required=True, choices=SOURCES)
    ap.add_argument('--dump', required=True, type=Path)
    ap.add_argument('--output', required=True, type=Path)
    ap.add_argument('--objdump', default='mips-linux-gnu-objdump')
    args = ap.parse_args()
    try:
        result = archive(args)
    except (OSError, ValueError, lzma.LZMAError, subprocess.CalledProcessError) as exc:
        ap.exit(1, str(exc) + '\n')
    print('%s: archived %d kernel bytes, %d symbols, %d complete disassembly chunks' %
          (args.board, result['kernel_size'], result['symbol_count'],
           len(result['disassembly_chunks'])))


if __name__ == '__main__':
    main()
