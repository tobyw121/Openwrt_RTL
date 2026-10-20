#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Extract structural AC-calibration evidence from known OEM files only.

No factory values or full OEM executables are exported. The bounded MIPS
interpreter below only models these straight-line functions on synthetic
buffers; it does not execute firmware or access hardware.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

KNOWN = {
    'mw5': ('57e7e76e615c1f48308276aadc17071638d3cf909b8eaa28307c8581c6c9880a',
            0x403308, 0x40341c, 0x4018c0),
    'rd05': ('64947529a3bc3b630b82ac99e6aaad6b85fab82bb81f99c9fa643f8a92f8ff0a',
             0x10c48, 0x10dcc, None),
}
LIB_SHA = 'bc1f256b52b6d1e51ad4eb26d68031cbb40852527fbf65995182224e1d55e8a4'
OFFSETS = [35, 40, 48, 56, 64, 104, 112, 120, 128, 136, 144, 153, 161, 169, 177]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def elf_offset(blob, address, size):
    if blob[:7] != b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H', blob, 18)[0] != 8:
        raise ValueError('requires little-endian ELF32 MIPS')
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', blob)
    for i in range(header[10]):
        kind, offset, base, _, count, _, _, _ = struct.unpack_from(
            '<8I', blob, header[5] + i * header[9])
        if kind == 1 and base <= address and address + size <= base + count:
            return offset + address - base
    raise ValueError('address outside file-backed ELF segment')


def rd05_memset_import(blob):
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', blob)
    dynamic = {}
    for i in range(header[10]):
        kind, offset, _, _, count, _, _, _ = struct.unpack_from(
            '<8I', blob, header[5] + i * header[9])
        if kind != 2:
            continue
        for pos in range(offset, offset + count, 8):
            tag, value = struct.unpack_from('<II', blob, pos)
            if not tag:
                break
            dynamic[tag] = value
    count = dynamic[0x70000011]  # DT_MIPS_SYMTABNO
    symbols = elf_offset(blob, dynamic[6], count * 16)
    strings = elf_offset(blob, dynamic[5], dynamic[10])
    for i in range(count):
        name = struct.unpack_from('<I', blob, symbols + i * 16)[0]
        text = blob[strings + name:strings + dynamic[10]].split(b'\0', 1)[0]
        if text != b'memset':
            continue
        # DT_PLTGOT + (LOCAL_GOTNO + symbol - GOTSYM) * sizeof(u32).
        got = dynamic[3] + 4 * (dynamic[0x7000000a] + i - dynamic[0x70000013])
        if got != 0x2e63c:
            raise ValueError('unexpected RD05 memset GOT slot')
        return {'symbol': 'memset', 'symbol_index': i, 'got_address': got,
                'gp_relative_offset': -32532}
    raise ValueError('missing memset import')


def interpret(words, board, groups=None):
    """Model one fixed straight-line routine, allowing only bounded writes."""
    groups = bytes(range(0xa0, 0xae)) if groups is None else bytes(groups)
    if len(groups) != 14 or len(words) > 128:
        raise ValueError('invalid model bounds')
    regs = [0] * 32
    regs[4], regs[5], regs[29], regs[25] = 0x10000000, 0x10001000, 0x20010000, KNOWN[board][1]
    target = bytearray(b'\x5a' * 196)
    stack = {}
    pending = None
    calls = 0

    def memset_call():
        nonlocal calls
        offset, value, count = regs[4] - 0x10000000, regs[5], regs[6]
        if not (0 <= offset < offset + count <= 196 and value in groups):
            raise ValueError('unbounded or non-synthetic memset')
        target[offset:offset + count] = bytes([value & 255]) * count
        calls += 1
        # Call-clobbered registers cannot be reused as evidence.
        for reg in (*range(2, 16), 24, 25, 31):
            regs[reg] = 0

    for i, word in enumerate(words):
        after_delay = pending
        pending = None
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        rd, imm = (word >> 11) & 31, word & 65535
        signed = imm if imm < 32768 else imm - 65536
        address = (regs[rs] + signed) & 0xffffffff
        if word == 0:
            pass
        elif op == 9:
            regs[rt] = address
        elif op == 15:
            regs[rt] = imm << 16
        elif op == 0 and word & 63 in (0x21, 0x25):
            regs[rd] = ((regs[rs] + regs[rt]) if word & 63 == 0x21
                        else regs[rs] | regs[rt]) & 0xffffffff
        elif op == 43:
            if not 0x2000ff00 <= address < 0x20010000:
                raise ValueError('store outside modeled stack')
            stack[address] = regs[rt]
        elif op == 35:
            if address in stack:
                regs[rt] = stack[address]
            elif board == 'rd05' and word == 0x8f9980ec:
                # This exact routine always loads the same imported memset
                # GOT slot before its indirect calls. No arbitrary target.
                if address != 0x2e63c:
                    raise ValueError('load does not address memset GOT slot')
                regs[rt] = 0x12345678
            else:
                raise ValueError('unmodeled load')
        elif op == 36:
            offset = address - 0x10001000
            if not 0 <= offset < 14:
                raise ValueError('load outside synthetic group array')
            regs[rt] = groups[offset]
        elif op == 40:
            offset = address - 0x10000000
            if not 0 <= offset < 196:
                raise ValueError('store outside synthetic target')
            target[offset] = regs[rt] & 255
        elif op in (2, 3):
            pc = KNOWN[board][1] + i * 4
            destination = ((pc + 4) & 0xf0000000) | ((word & 0x3ffffff) << 2)
            if destination != KNOWN[board][3]:
                raise ValueError('unexpected direct target')
            pending = 'return' if op == 2 else 'call'
        elif op == 0 and word in (0x0320f809, 0x03200008):
            if regs[25] != 0x12345678:
                raise ValueError('unexpected indirect target')
            pending = 'return' if word == 0x03200008 else 'call'
        else:
            raise ValueError('unsupported instruction: %08x' % word)
        regs[0] = 0
        if after_delay:
            if pending:
                raise ValueError('branch in delay slot')
            memset_call()
            if after_delay == 'return':
                if i != len(words) - 1:
                    raise ValueError('unexpected bytes after tail call')
                if calls != (13 if board == 'mw5' else 14):
                    raise ValueError('unexpected memset count')
                return bytes(target)
    raise ValueError('missing bounded tail call')


def collect(mw5_flash, rd05_flash, rd05_library):
    result = {'schema': 1, 'offsets_zero_based': OFFSETS,
              'limits': 'Synthetic bounded MIPS model and structural metadata only; not RF or hardware validation.',
              'functions': {}, 'rd05_fields': []}
    result['sdk'] = {'commit': 'e6d68e9067f54b695022f61ed52e50dc56c5a7b7',
                     'path': 'rtl819x/users/boa/utils/flash.c',
                     'sha256': '5ffcae1d73b124a8e46bac5cc0e1a6b97fb38887b3aa977ea883a00b64ef85af',
                     'function': 'assign_diff_AC'}
    for board, path in (('mw5', mw5_flash), ('rd05', rd05_flash)):
        blob = path.read_bytes()
        sha, start, stop, _ = KNOWN[board]
        if digest(blob) != sha:
            raise ValueError('unknown OEM utility: ' + board)
        offset = elf_offset(blob, start, stop - start)
        code = blob[offset:offset + stop - start]
        words = struct.unpack('<%dI' % (len(code) // 4), code)
        actual = interpret(words, board)
        expected = bytearray(b'\x5a' * 196)
        for i, (lo, hi) in enumerate(zip(OFFSETS, OFFSETS[1:])):
            expected[lo:hi] = bytes([0xa0 + i]) * (hi - lo)
        if actual != expected:
            raise ValueError('OEM group map differs from SDK')
        result['functions'][board] = {
            'oem_path': '/bin/flash' if board == 'mw5' else '/usr/sbin/flash',
            'file_sha256': sha, 'address': start, 'file_offset': offset,
            'size': len(code), 'span_sha256': digest(code),
            'instruction_words': ['0x%08x' % w for w in words],
        }
        if board == 'rd05':
            result['functions'][board]['memset_import'] = rd05_memset_import(blob)
    library = rd05_library.read_bytes()
    if digest(library) != LIB_SHA:
        raise ValueError('unknown RD05 APMIB library')
    table = elf_offset(library, 0x1f880, 5916)
    result['rd05_library'] = {'sha256': LIB_SHA, 'table_address': 0x1f880,
                              'table_size': 5916, 'record_stride': 68}
    for pos in range(table, table + 5916, 68):
        name = library[pos + 4:pos + 44].split(b'\0')[0].decode('ascii')
        if not name.startswith('TX_POWER_DIFF_') or name.endswith(('3S_A', '4S_A', '3S_B', '4S_B')):
            continue
        kind, offset, size = struct.unpack_from('<III', library, pos + 44)
        if kind != 8 or size != 14:
            continue
        if (name.startswith(('TX_POWER_DIFF_20BW1S_', 'TX_POWER_DIFF_40BW2S_')) or
            name.startswith(('TX_POWER_DIFF_5G_20BW1S_', 'TX_POWER_DIFF_5G_40BW2S_',
                             'TX_POWER_DIFF_5G_80BW1S_', 'TX_POWER_DIFF_5G_80BW2S_'))):
            result['rd05_fields'].append({'key': name, 'offset': offset, 'size': size,
                                          'record_file_offset': pos})
    if len(result['rd05_fields']) != 12:
        raise ValueError('unexpected APMIB field count')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mw5-flash', type=Path, required=True)
    parser.add_argument('--rd05-flash', type=Path, required=True)
    parser.add_argument('--rd05-apmib', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = collect(args.mw5_flash, args.rd05_flash, args.rd05_apmib)
    with args.output.open('x') as output:
        json.dump(result, output, indent=2)
        output.write('\n')
    print('Verified both OEM group maps and twelve RD05 fields; exported no factory values')


if __name__ == '__main__':
    main()
