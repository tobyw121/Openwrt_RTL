#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Reproduce extension-mode and WLAN key-path evidence from kernel archives.

Only hashes, symbol metadata, call sites and a short instruction proof are
exported. Static call sites do not establish execution or hardware behavior.
"""
import argparse
import hashlib
import importlib.util
import json
import lzma
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    'switch_audit', Path(__file__).with_name('rtl8197f-oem-switch-audit.py'))
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)
NAMES = ('rtl8367c_setAsicPortExtMode', 'rtl_net80211_setkey',
         'rtl_net80211_delkey', 'CamDeleteOneEntry', 'CamResetAllEntry',
         'DOT11_Process_Set_Key', 'hal_Associate_8812FE', 'StopHW8812FE')
NAMES += ('dal_rtl8367d_switch_init',)
MTU_NAMES = ('rtk_switch_maxPktLen_set', 'rtl8367c_setAsicMaxLength',
             'rtl8367c_setAsicMaxLengthCfg',
             'dal_rtl8367d_switch_portMaxPktLen_set',
             'dal_rtl8367d_switch_maxPktLenCfg_set')
NAMES += MTU_NAMES


def verified_artifact(directory, manifest, name):
    packed = (directory / name).read_bytes()
    entry = manifest['artifacts'][name]
    if len(packed) != entry['size'] or hashlib.sha256(packed).hexdigest() != entry['sha256']:
        raise ValueError('archive hash mismatch: ' + name)
    plain = lzma.decompress(packed)
    if (len(plain) != entry['unpacked_size'] or
            hashlib.sha256(plain).hexdigest() != entry['unpacked_sha256']):
        raise ValueError('unpacked archive hash mismatch: ' + name)
    return plain


def mode_mask_proofs(kernel, base, function, setter):
    start = function['address']
    offset = start - base
    words = struct.unpack('<%dI' % (function['span'] // 4),
                          kernel[offset:offset + function['span']])
    proofs = []
    for i, word in enumerate(words):
        # sll r,r,2; then li a1,15; sllv a1,a1,r; jal setter.
        reg = (word >> 16) & 31
        if not reg or word != ((reg << 16) | (reg << 11) | (2 << 6)):
            continue
        for j in range(i + 1, min(i + 6, len(words) - 2)):
            pc = start + j * 4
            target = ((pc + 4) & 0xf0000000) | ((words[j] & 0x3ffffff) << 2)
            if words[j] >> 26 != 3 or target != setter:
                continue
            window = words[i:j + 2]
            shift = (reg << 21) | (5 << 16) | (5 << 11) | 4
            if (0x2405000f not in window or shift not in window or
                    0x24041305 not in window):
                continue
            # Accept only the straight-line register setup, JAL and delay
            # slot. Never propagate a value across a branch/unknown write.
            mode_move = 0x02003021 if reg == 17 else 0x02203025 if reg == 16 else None
            setup = {word, 0x2405000f, shift, 0x24041305,
                     mode_move, words[j]}
            if len(window) != 6 or set(window) != setup:
                continue
            if window.index(0x2405000f) > window.index(shift):
                continue
            proofs.append({'pc': start + i * 4, 'call_pc': pc,
                           'setter': setter, 'register': 0x1305,
                           'mask_expression': '0xf << (extension_id * 4)',
                           'instruction_words': ['0x%08x' % w for w in window]})
    if not proofs:
        raise ValueError('no exact extension-mask proof found')
    return proofs


def collect(archives):
    result = {
        'schema': 1,
        'scope': 'Static OEM function spans, direct JAL sites and extension-mode mask proof; no factory values. RD05 C-family routines are not proof of execution on its D-family path.',
        'sdk': {'commit': 'e6d68e9067f54b695022f61ed52e50dc56c5a7b7',
                'path': '8197/API_Source/rtl8367c_asicdrv_port.c',
                'sha256': 'd79bb85801d2881b6a502733a536e2e8b3eb88bbc0ee5729cb4026d1f4133123',
                'expression': 'RTL8367C_SELECT_GMII_0_MASK << (id * RTL8367C_SELECT_GMII_1_OFFSET)',
                'rd05_init_path': '8197/RT8197F_RTL8367RB_Gesamtpaket/AX12v1_GPL_Paket/source_subset/rtl8197/rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/dal/rtl8367d/dal_rtl8367d_switch.c',
                'rd05_init_sha256': '3c9e0430495ca3c1c650a7299fee0097f1b2bfb70531746f1e20203f7285b3b7'},
        'boards': {},
    }
    for board in ('mw5', 'ac23', 'rd05'):
        directory = archives / board
        manifest = json.loads((directory / 'manifest.json').read_text())
        kernel = verified_artifact(directory, manifest, 'kernel.raw.xz')
        rows = verified_artifact(directory, manifest, 'symbols.tsv.xz').decode().splitlines()
        symbols = []
        if rows[0] != 'address\ttype\tname':
            raise ValueError('unexpected symbol table header')
        for row in rows[1:]:
            address, kind, name = row.split('\t')
            symbols.append({'address': int(address, 16), 'type': kind, 'name': name})
        metadata = {'image_base': manifest['image_base'], 'symbols': symbols}
        functions = AUDIT.audit_functions(kernel, metadata, NAMES)
        for name in MTU_NAMES:
            if name in functions:
                item = functions[name]
                offset = item['address'] - manifest['image_base']
                code = kernel[offset:offset + item['span']]
                item['instruction_words'] = ['0x%08x' % w for w in
                    struct.unpack('<%dI' % (len(code) // 4), code)]
        setter = next(s['address'] for s in symbols if s['name'] == 'rtl8367c_setAsicRegBits')
        proofs = mode_mask_proofs(kernel, manifest['image_base'],
                                 functions[NAMES[0]], setter)
        # Keep the mode routine's full hash and proof, rather than a long
        # listing of unrelated SerDes branches and their register writes.
        mode = functions[NAMES[0]]
        mode.pop('direct_calls')
        mode.pop('literal_call_sites')
        result['boards'][board] = {
            'kernel_sha256': manifest['kernel_sha256'],
            'functions': functions, 'extension_mode_proofs': proofs,
        }
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archives', type=Path,
                        default=ROOT / 'reverse-engineering/oem-kernels')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    data = collect(args.archives)
    with args.output.open('x') as stream:
        json.dump(data, stream, indent=2)
        stream.write('\n')
    print('Verified static interface evidence for MW5, AC23 and RD05')


if __name__ == '__main__':
    main()
