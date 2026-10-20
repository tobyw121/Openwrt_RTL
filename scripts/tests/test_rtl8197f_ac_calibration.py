#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compare the real RF importer with bounded models of both OEM routines."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT, WLAN
from test_rtl8197f_factory import factory_program

SPEC = importlib.util.spec_from_file_location(
    'ac_calibration', ROOT / 'scripts/rtl8197f-oem-ac-calibration.py')
MODEL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODEL)
EVIDENCE = ROOT / 'target/linux/realtek/rtl8197f/oem-ac-calibration-evidence.json'


class AcCalibrationTests(unittest.TestCase):
    def test_oem_word_hashes_maps_and_fail_closed_model(self):
        data = json.loads(EVIDENCE.read_text())
        self.assertEqual(data['offsets_zero_based'], MODEL.OFFSETS)
        results = []
        for board, function in data['functions'].items():
            with self.subTest(board=board):
                words = [int(word, 16) for word in function['instruction_words']]
                code = struct.pack('<%dI' % len(words), *words)
                self.assertEqual(hashlib.sha256(code).hexdigest(), function['span_sha256'])
                self.assertEqual(len(code), function['size'])
                mapped = []
                for n in range(256):
                    values = bytes((n + 17 * i) & 255 for i in range(14))
                    actual = MODEL.interpret(words, board, values)
                    expected = bytearray(b'\x5a' * 196)
                    for i, (low, high) in enumerate(zip(MODEL.OFFSETS, MODEL.OFFSETS[1:])):
                        expected[low:high] = bytes([values[i]]) * (high - low)
                    self.assertEqual(actual, expected)
                    mapped.append(actual)
                results.append(mapped)
                with self.assertRaisesRegex(ValueError, 'unsupported'):
                    MODEL.interpret([0x10000001] + words[1:], board)
                with self.assertRaisesRegex(ValueError, 'bounds'):
                    MODEL.interpret(words, board, b'\0' * 13)
        self.assertEqual(results[0], results[1])
        self.assertEqual(len(data['rd05_fields']), 12)

    def test_actual_importer_matches_oem_for_kv_and_rd05_binary(self):
        data = json.loads(EVIDENCE.read_text())
        header = (WLAN / 'linux6/rtl8192cd_factory.h').read_text()
        fields = re.findall(r'RF_FIELD\("([A-Z0-9_]+)", (\w+), (0x[0-9a-f]+), '
                            r'(\d+), (\d+), (\d+), 0\)', header)
        grouped = [f for f in fields if f[5] == '3']
        self.assertEqual(len(grouped), 8)
        words = [int(w, 16) for w in data['functions']['rd05']['instruction_words']]
        values = bytes(range(0xf0, 0xfe))
        expected = MODEL.interpret(words, 'rd05', values)
        program = factory_program() + r'''
static struct Dot11RFEntry rf;
static struct rtl_factory_stats stats;
static unsigned char blob[65536];
static const unsigned char expected[196]={EXPECTED};
static void checksum(void)
{
    unsigned sum=0;
    for (unsigned i=6;i<6+6047;i++) sum+=blob[i];
    blob[6+6047]=(unsigned char)-sum;
}
static void kv(const char *text, unsigned radio, bool band5)
{
    assert(!rtl_factory_apply(&rf,RTL_FACTORY_TENDA_KV,radio,4,band5,
                              (const u8 *)text,strlen(text)+1,&stats));
}
int main(void)
{
'''.replace('EXPECTED', ','.join(str(x) for x in expected))
        for key, member, offset, _, _, _ in grouped:
            record = 'HW_WLAN0_' + key + '=' + values.hex() + '\n'
            program += f'''
    memset(&rf,0x5a,sizeof(rf));
    kv({json.dumps(record)},0,true);
    assert(stats.applied==1 && stats.rejected==0);
    assert(!memcmp(rf.{member},expected,196));
    memset(&rf,0x5a,sizeof(rf));
    kv({json.dumps(record)},1,true);
    assert(stats.applied==0 && stats.rejected==0);
    assert(rf.{member}[35]==0x5a);
    kv({json.dumps(record)},0,false);
    assert(stats.applied==0 && stats.rejected==0);

    memset(blob,0xff,sizeof(blob));
    memcpy(blob,"H601",4);blob[4]=0x17;blob[5]=0xa0;
    for(unsigned i=0;i<14;i++) blob[6+0x409+{offset}+i]=0xf0+i;
    checksum();memset(&rf,0x5a,sizeof(rf));
    assert(!rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,0,2,true,
                              blob,sizeof(blob),&stats));
    assert(!memcmp(rf.{member},expected,196));
'''
        # A malformed new group record must leave the complete RF MIB intact.
        key, member = grouped[0][:2]
        prefix = 'HW_WLAN0_' + key + '='
        invalid = [prefix + '00' * 13 + '\n', prefix + '00' * 15 + '\n',
                   prefix + '00' * 196 + '\n', prefix + '00' * 13 + 'zz\n',
                   prefix + 'ff' * 14 + '\n',
                   (prefix + values.hex() + '\n') * 2]
        for record in invalid:
            program += f'''
    memset(&rf,0x5a,sizeof(rf));
    {{ struct Dot11RFEntry before=rf;
       kv({json.dumps(record)},0,true);
       assert(stats.applied==0 && stats.rejected==1);
       assert(!memcmp(&before,&rf,sizeof(rf))); }}
'''
        program += 'return 0;\n}\n'
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'ac.c'
            exe = Path(tmp) / 'ac'
            src.write_text(program)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(WLAN), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
