#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Execute the real RF importer against malformed and valid synthetic factories."""
from pathlib import Path
import json
import re
import subprocess
import tempfile
import unittest

from test_mw5_stability import WLAN


def factory_program():
    mib = (WLAN / 'ieee802_mib.h').read_text()
    start = mib.index('struct Dot11RFEntry {')
    end = mib.index('\n};', start) + 3
    return r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define MAX_2G_CHANNEL_NUM 14
#define MAX_5G_CHANNEL_NUM 196
''' + mib[start:end] + '\n#include "linux6/rtl8192cd_factory.h"\n'


class FactoryCalibrationTests(unittest.TestCase):
    def test_rd05_offsets_match_recovered_oem_library_metadata(self):
        schema = json.loads((WLAN.parents[3] /
                             'target/linux/realtek/rtl8197f/oem-calibration-schema.json').read_text())
        rd05 = schema['boards']['rd05']
        self.assertEqual(rd05['hw_setting_size'], 6047)
        self.assertEqual(rd05['wlan_root_offset'], 0x409)
        self.assertEqual(rd05['radio_stride'], 2507)
        evidence = {field['key']: field for field in rd05['fields']}
        header = (WLAN / 'linux6/rtl8192cd_factory.h').read_text()
        fields = re.findall(r'RF_FIELD\("([A-Z0-9_]+)", [^,]+, (0x[0-9a-f]+),', header)
        self.assertGreater(len(fields), 20)
        for key, offset in fields:
            with self.subTest(key=key):
                self.assertEqual(int(offset, 16), evidence[key]['offset'])
                self.assertLessEqual(evidence[key]['offset'] + evidence[key]['size'],
                                     rd05['radio_stride'])

    def test_valid_corrupt_scoped_and_binary_calibration(self):
        program = factory_program() + r'''
static struct Dot11RFEntry rf;
static struct rtl_factory_stats stats;
static unsigned char blob[65536];

static void reset(void)
{
    memset(&rf, 0x5a, sizeof(rf));
    rf.tssi_enable = 0;
    memset(blob, 0xff, sizeof(blob));
}
static void kv(const char *text, unsigned radio, unsigned paths, bool band5)
{
    assert(rtl_factory_apply(&rf, RTL_FACTORY_TENDA_KV, radio, paths,
                             band5, (const u8 *)text, strlen(text)+1, &stats)==0);
}
static void checksum(void)
{
    unsigned sum=0;
    for(unsigned i=6;i<6+6047;i++) sum+=blob[i];
    blob[6+6047]=(unsigned char)-sum;
}
static void array_record(char *out, size_t capacity, unsigned radio,
                         const char *name, unsigned count, unsigned value)
{
    size_t used=strlen(out);
    int n=snprintf(out+used,capacity-used,"HW_WLAN%u_%s=",radio,name);
    assert(n>0 && (size_t)n<capacity-used);
    used+=n;
    for(unsigned i=0;i<count;i++) {
        assert(used+3<capacity);
        used+=snprintf(out+used,capacity-used,"%02x",value);
    }
    out[used++]='\n'; out[used]=0;
}
int main(void)
{
    reset();
    kv("HW_WLAN1_11N_XCAP=42\nHW_WLAN0_11N_XCAP=13\n"
       "HW_WLAN1_TX_POWER_CCK_A=0102030405060708090a0b0c0d0e\n",1,2,false);
    assert(rf.xcap==42); /* decimal, not hex 0x42 */
    assert(rf.pwrlevelCCK_A[0]==1 && rf.pwrlevelCCK_A[13]==14);
    assert(rf.pwrlevelCCK_B[0]==0x5a);
    assert(rf.pwrlevel5GHT40_1S_A[0]==0x5a);
    assert(rf.rfe_type==0x5a);
    assert(stats.rejected==0);

    reset();
    kv("HW_WLAN1_11N_XCAP=42\nHW_WLAN1_11N_XCAP=43\n"
       "HW_WLAN1_TX_POWER_CCK_A=0102030405060708090a0b0c0dz0\n",1,2,false);
    assert(rf.xcap==0x5a5a5a5a && stats.rejected==2);
    for(unsigned i=0;i<14;i++) assert(rf.pwrlevelCCK_A[i]==0x5a);
    kv("prefixHW_WLAN1_11N_XCAP=42\n",1,2,false);
    assert(stats.applied==0);
    kv("HW_WLAN1_11N_XCAP=-1\n",1,2,false);
    assert(stats.rejected==1 && rf.xcap==0x5a5a5a5a);
    kv("HW_WLAN1_11N_XCAP=64\n",1,2,false);
    assert(stats.rejected==1);
    kv("HW_WLAN1_11N_XCAP=0x2a\n",1,2,false);
    assert(stats.rejected==1);
    const char *unterminated="HW_WLAN1_11N_XCAP=42";
    assert(!rtl_factory_apply(&rf,RTL_FACTORY_TENDA_KV,1,2,false,
                             (const u8 *)unterminated,strlen(unterminated),&stats));
    assert(stats.rejected==1);
    kv("HW_WLAN1_TX_POWER_CCK_A=ffffffffffffffffffffffffffff\n",1,2,false);
    assert(stats.rejected==1 && rf.pwrlevelCCK_A[0]==0x5a);

    char text[8192]={0};
    reset();
    array_record(text,sizeof(text),0,"TX_POWER_5G_HT40_1S_A",177,0x21);
    kv(text,0,2,true);
    assert(stats.rejected==0 && rf.pwrlevel5GHT40_1S_A[176]==0x21);
    assert(rf.pwrlevel5GHT40_1S_A[177]==0x5a);
    text[0]=0;
    array_record(text,sizeof(text),0,"TX_POWER_5G_HT40_1S_B",176,0x21);
    kv(text,0,2,true);
    assert(stats.rejected==1 && rf.pwrlevel5GHT40_1S_B[0]==0x5a);

    reset();text[0]=0;
    array_record(text,sizeof(text),0,"TX_POWER_TSSI_5G_HT40_1S_A",196,0x23);
    strcat(text,"HW_WLAN0_11N_TSSI_ENABLE=1\n");
    kv(text,0,4,true);
    assert(rf.tssi_enable==0 && stats.rejected==1);
    text[0]=0;
    const char *names[]={"TX_POWER_TSSI_5G_HT40_1S_A", "TX_POWER_TSSI_5G_HT40_1S_B",
                         "TX_POWER_TSSI_5G_HT40_1S_C", "TX_POWER_TSSI_5G_HT40_1S_D"};
    for(unsigned i=0;i<4;i++) array_record(text,sizeof(text),0,names[i],196,0x24+i);
    strcat(text,"HW_WLAN0_11N_TSSI_ENABLE=1\nHW_WLAN0_11N_THER_4=33\n");
    kv(text,0,4,true);
    assert(stats.rejected==0 && rf.tssi_enable==1 && rf.thermal[3]==33);
    assert(rf.pwrlevel_TSSI5GHT40_1S_A[195]==0x24);
    assert(rf.pwrlevel_TSSI5GHT40_1S_D[195]==0x27);
    reset();kv(text,0,2,true);
    assert(rf.tssi_enable==1 && rf.pwrlevel_TSSI5GHT40_1S_C[0]==0x5a);
    assert(rf.thermal[3]==0x5a5a5a5a);

    reset();text[0]=0;
    array_record(text,sizeof(text),0,"TX_POWER_TSSI_5G_HT40_1S_A",177,0x24);
    array_record(text,sizeof(text),0,"TX_POWER_TSSI_5G_HT40_1S_B",177,0x25);
    strcat(text,"HW_WLAN0_11N_TSSI_ENABLE=1\n");
    kv(text,0,2,true);
    assert(stats.rejected==3 && rf.tssi_enable==0);
    assert(rf.pwrlevel_TSSI5GHT40_1S_A[0]==0x5a);

    /* Exact RD05 H601 schema; two radio records must not alias. */
    reset();memset(blob+6,0,6048);
    memcpy(blob,"H601",4);blob[4]=0x17;blob[5]=0xa0;
    size_t r0=6+0x409, r1=r0+2507;
    blob[r0+0x9d]=21;blob[r1+0x9d]=38;
    memset(blob+r0+0xad,0x31,196);
    memset(blob+r1+0x30,0x32,14);
    checksum();
    assert(!rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,0,2,true,blob,sizeof(blob),&stats));
    assert(rf.xcap==21 && rf.pwrlevel5GHT40_1S_A[195]==0x31);
    assert(!rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,1,2,false,blob,sizeof(blob),&stats));
    assert(rf.xcap==38 && rf.pwrlevelCCK_A[13]==0x32);
    struct Dot11RFEntry before=rf;
    blob[r0+0xad]^=1;
    assert(rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,0,2,true,blob,sizeof(blob),&stats)<0);
    assert(!memcmp(&before,&rf,sizeof(rf)));
    blob[r0+0xad]^=1;
    assert(rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,0,2,true,blob,6048,&stats)<0);
    blob[5]^=1;
    assert(rtl_factory_apply(&rf,RTL_FACTORY_RD05_H601,0,2,true,blob,sizeof(blob),&stats)<0);
    assert(!memcmp(&before,&rf,sizeof(rf)));
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'factory.c'
            exe = Path(tmp) / 'factory'
            src.write_text(program)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(WLAN), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
