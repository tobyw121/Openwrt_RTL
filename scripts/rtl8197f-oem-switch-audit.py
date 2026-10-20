#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Audit named MIPS32LE switch functions without executing OEM firmware.

Input is a decompressed OEM kernel with validated absolute kallsyms.
Call sites describe static code, not executed paths or successful writes.
Only fully literal argument windows are decoded, including the delay slot.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct


DEFAULT_SYMBOLS = (
    'RTL83XX_init', 'rtk_switch_init', 'rtk_switch_probe', 'switch_probe',
    'hal_find_device', 'dal_mgmt_attachDevice', 'dal_rtl8367d_mapper_get',
    'dal_rtl8367d_switch_init', 'rtk_switch_port_L2P_get',
    'rtk_port_rgmiiDelayExt_set', 'rtk_port_macForceLinkExtSSC_set',
    'rtk_phyPatch_set',
)
LITERAL_TARGETS = {
    'rtl8367c_setAsicReg': (4, 5),
    'rtl8367d_setAsicReg': (4, 5),
    'rtk_port_rgmiiDelayExt_set': (4, 5, 6),
}


def literal(word):
    """Return (register, u32 constant) for a single immediate/zero move."""
    op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
    value = word & 0xffff
    if op in (9, 13) and rs == 0:  # addiu / ori from zero
        if op == 9 and value & 0x8000:
            value -= 0x10000
        return rt, value & 0xffffffff
    if (op == 0 and rs == rt == 0 and not (word & 0x7c0) and
            (word & 63) in (0x21, 0x25)):
        return (word >> 11) & 31, 0  # addu / or from zero
    return None


def audit_functions(blob, recovered, names):
    base = recovered['image_base']
    symbols = recovered['symbols']
    by_name = {s['name']: s for s in symbols}
    by_addr = {s['address']: s['name'] for s in symbols if s['type'] in 'Tt'}
    addresses = sorted(set(s['address'] for s in symbols))
    result = {}
    for name in names:
        if name not in by_name:
            continue
        symbol = by_name[name]
        if symbol['type'] not in 'Tt':
            raise ValueError('not a text symbol: ' + name)
        start = symbol['address']
        stop = next((a for a in addresses if a > start), base + len(blob))
        if not (base <= start < stop <= base + len(blob) and
                start % 4 == stop % 4 == 0 and stop - start <= 128 * 1024):
            raise ValueError('invalid or oversized function range: ' + name)
        code = blob[start - base:stop - base]
        words = struct.unpack('<%dI' % (len(code) // 4), code)
        item = {'address': start, 'span': len(code),
                'span_sha256': hashlib.sha256(code).hexdigest(),
                'direct_calls': [], 'literal_call_sites': []}
        for i, word in enumerate(words):
            if word >> 26 != 3:  # JAL, whose high bits come from PC+4
                continue
            pc = start + i * 4
            target = ((pc + 4) & 0xf0000000) | ((word & 0x3ffffff) << 2)
            target_name = by_addr.get(target)
            item['direct_calls'].append({'pc': pc, 'target': target,
                                         'symbol': target_name})
            required = LITERAL_TARGETS.get(target_name)
            if not required or i < len(required) - 1 or i + 1 >= len(words):
                continue
            # Exactly one setter for each argument; no intervening unknown
            # operation is accepted. Keep the raw call and delay-slot proof.
            before = words[i - len(required) + 1:i]
            decoded = [literal(w) for w in (*before, words[i + 1])]
            if any(v is None for v in decoded):
                continue
            values = dict(decoded)
            if set(values) != set(required):
                continue
            item['literal_call_sites'].append({
                'call_pc': pc, 'target': target_name,
                'arguments': [values[r] for r in required],
                'instruction_words': ['0x%08x' % w for w in
                                      (*before, word, words[i + 1])],
            })
        # An exact two-instruction success stub, with optional alignment NOPs.
        if (len(words) >= 2 and words[0] == 0x03e00008 and
                literal(words[1]) == (2, 0) and not any(words[2:])):
            item['constant_return'] = {
                'value': 0, 'instruction_words': ['0x%08x' % w for w in words]}
        result[name] = item
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('kernel', type=Path)
    ap.add_argument('--output', required=True, type=Path)
    ap.add_argument('--symbol', action='append', help='text symbol; may be repeated')
    args = ap.parse_args()
    decoder_path = Path(__file__).with_name('rtl8197f-oem-kallsyms.py')
    spec = importlib.util.spec_from_file_location('oem_kallsyms', decoder_path)
    decoder = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(decoder)
    try:
        blob = args.kernel.read_bytes()
        recovered = decoder.recover(blob)
        result = {'schema': 1, 'kernel_sha256': recovered['kernel_sha256'],
                  'limits': 'Static call sites only; no reachability, runtime '
                            'success or complete initialization is established.',
                  'functions': audit_functions(blob, recovered,
                                               args.symbol or DEFAULT_SYMBOLS)}
        if not result['functions']:
            raise ValueError('no requested text symbols found')
        with args.output.open('x') as output:
            json.dump(result, output, indent=2)
            output.write('\n')
    except (OSError, ValueError) as exc:
        ap.exit(1, str(exc) + '\n')
    print('Audited %d switch functions; metadata written to %s' %
          (len(result['functions']), args.output))


if __name__ == '__main__':
    main()
