#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Recover absolute MIPS32 little-endian kallsyms from decompressed OEM kernels.

Supports the Linux 3.10/4.4 representation found in the MW5/AC23/RD05 dumps.
It does not execute firmware or extract arbitrary strings/factory settings.
Input is the decompressed kernel, not an SPI dump or a compressed image.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct


IDENTIFIER = re.compile(rb'[A-Za-z0-9_.$]+')


def pointer_runs(blob):
    words = struct.unpack_from('<%dI' % (len(blob) // 4), blob)
    runs = []
    start = 0
    previous = 0
    for i, value in enumerate(words):
        if not (0x80000000 <= value < 0x88000000 and
                (i == start or value >= previous)):
            if i - start >= 1000:
                runs.append((start * 4, words[start:i]))
            start = i + 1
        previous = value
    return runs


def token_tables(blob):
    for match in re.finditer(rb'\x00\x00[\x01-\x20]\x00', blob):
        pos = match.start()
        if pos % 2 or pos + 512 > len(blob):
            continue
        offsets = struct.unpack_from('<256H', blob, pos)
        if not (400 < offsets[-1] < 4096 and
                all(0 < offsets[i + 1] - offsets[i] <= 64 for i in range(255))):
            continue
        for start in range(max(0, pos - offsets[-1] - 80), pos - offsets[-1] - 1):
            tokens = []
            for i, offset in enumerate(offsets):
                p = start + offset
                if (i and blob[p - 1]) or p >= pos:
                    break
                end = blob.find(b'\0', p, pos)
                if end < p or end - p > 64 or any(not 32 <= c < 127 for c in blob[p:end]):
                    break
                tokens.append(blob[p:end])
            if len(tokens) == 256 and not any(blob[end + 1:pos]):
                yield start, pos, tokens


def decode_names(blob, start, limit, count, tokens):
    pos = start
    names = []
    markers = []
    for i in range(count):
        if i % 256 == 0:
            markers.append(pos - start)
        if pos >= limit:
            return None
        length = blob[pos]
        pos += 1
        if not length or pos + length > limit:
            return None
        name = b''.join(tokens[v] for v in blob[pos:pos + length])
        pos += length
        if (len(name) < 2 or name[:1] not in b'TtDdRrBbAaVvWwNnUuSsGgIi?' or
                not IDENTIFIER.fullmatch(name[1:])):
            return None
        names.append(name.decode('ascii'))
    # Linker alignment differs between these OEM builds (4/8/16 bytes).
    for marker_start in range((pos + 3) & ~3, ((pos + 3) & ~3) + 32, 4):
        marker_end = marker_start + len(markers) * 4
        if marker_end > limit or limit - marker_end > 32:
            continue
        actual = struct.unpack_from('<%dI' % len(markers), blob, marker_start)
        if tuple(markers) == actual and not any(blob[marker_end:limit]):
            return names, marker_start
    return None


def recover(blob):
    if len(blob) > 64 * 1024 * 1024:
        raise ValueError('kernel exceeds the supported 64 MiB bound')
    results = []
    for token_start, token_index, tokens in token_tables(blob):
        for address_start, addresses in pointer_runs(blob):
            count = len(addresses)
            end = address_start + count * 4
            for num_pos in range(end, min(end + 32, len(blob) - 3), 4):
                if struct.unpack_from('<I', blob, num_pos)[0] != count:
                    continue
                for names_start in range(num_pos + 4, num_pos + 24, 4):
                    decoded = decode_names(blob, names_start, token_start, count, tokens)
                    if not decoded:
                        continue
                    names, markers = decoded
                    symbols = [{'address': a, 'type': n[0], 'name': n[1:]}
                               for a, n in zip(addresses, names)]
                    results.append({
                        'kernel_sha256': hashlib.sha256(blob).hexdigest(),
                        'symbol_count': count,
                        'image_base': 0x80000000,
                        'offsets': {'addresses': address_start, 'num_syms': num_pos,
                                    'names': names_start, 'markers': markers,
                                    'token_table': token_start, 'token_index': token_index},
                        'symbols': symbols,
                    })
    if len(results) != 1:
        raise ValueError('expected one fully validated kallsyms table, found %d' % len(results))
    result = results[0]
    by_name = {s['name']: s['address'] for s in result['symbols']}
    verified = mismatches = 0
    for symbol in result['symbols']:
        if not symbol['name'].startswith('__ksymtab_'):
            continue
        name = symbol['name'][len('__ksymtab_'):]
        string_symbol = '__kstrtab_' + name
        p = symbol['address'] - result['image_base']
        if name not in by_name or string_symbol not in by_name or not 0 <= p <= len(blob) - 8:
            continue
        value, name_ptr = struct.unpack_from('<II', blob, p)
        offset = name_ptr - result['image_base']
        if (value == by_name[name] and name_ptr == by_name[string_symbol] and
                0 <= offset < len(blob) and
                blob[offset:offset + len(name) + 1] == name.encode('ascii') + b'\0'):
            verified += 1
        else:
            mismatches += 1
    if mismatches:
        raise ValueError('exported-symbol cross-check failed: %d mismatches' % mismatches)
    result['export_cross_checks'] = verified
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('kernel', type=Path)
    ap.add_argument('--output', required=True, type=Path, help='JSON metadata/symbol output')
    ap.add_argument('--filter', default=r'^(?:RTL83XX_init|rtk_switch_|switch_phy_status_get|'
                    r'curLinkPortMask$|one_sec_timer$|rtl_get_8367r_port_link_status$|'
                    r'rtl_check_swCore_tx_hang$|New_check_tx_done_desc_swCore_own$|'
                    r'rtl865x_reinitSwitchCore$|rtl_check_tx_hang_flag$|'
                    r'rtl_check_swCore_tx_hang_interval$|rtl_reinit_swCore_threshold$|'
                    r'PatchChipData$|dal_rtl8367[cd]_init$)')
    ap.add_argument('--all-symbols', action='store_true')
    args = ap.parse_args()
    if args.kernel.resolve() == args.output.resolve():
        ap.error('input and output must differ')
    try:
        result = recover(args.kernel.read_bytes())
        if not args.all_symbols:
            select = re.compile(args.filter)
            result['symbols'] = [s for s in result['symbols'] if select.search(s['name'])]
        with args.output.open('x') as f:
            json.dump(result, f, indent=2)
            f.write('\n')
    except (ValueError, OSError) as exc:
        ap.exit(1, str(exc) + '\n')
    print('Recovered %d symbols, export cross-checks=%d; wrote %d selected symbols to %s' %
          (result['symbol_count'], result['export_cross_checks'], len(result['symbols']), args.output))


if __name__ == '__main__':
    main()
