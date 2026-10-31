#!/usr/bin/env python3
"""Execute the RD05 RGMII transaction with ordered regmap fault injection."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function


class RD05RGMIISequenceTests(unittest.TestCase):
    def test_quiesce_readback_errors_and_board_isolation(self):
        patch = ROOT / 'target/linux/realtek/patches-6.6/339-net-dsa-realtek-rd05-quiesce-and-check-rgmii.patch'
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'test.c'
            p.write_text('\n'.join(line[1:] for line in patch.read_text().splitlines()
                                   if line.startswith((' ', '+')) and not line.startswith('+++')))
            helper = c_function(p, 'static int rtl8367d_rd05_configure_link(')
            p.write_text(r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
typedef uint32_t u32;
#define READ_ONCE(x) (x)
#define FIELD_PREP(m,v) (((v) * ((m) & -(m))) & (m))
#define RTL8365MB_EXT_RGMXF_TXDELAY_MASK 8
#define RTL8365MB_EXT_RGMXF_RXDELAY_MASK 7
#define RTL8365MB_EXT_RGMXF_REG(i) (0x1306+(i))
#define RTL8367D_EXT_TXC_DLY_REG 0x13f9
#define RTL8367D_EXT1_RGMII_TX_DLY_MASK 0x38
#define SPEED_1000 1000
#define DUPLEX_FULL 1
#define dev_err(...) ((void)0)
struct rtl8365mb { bool rd05, family_d; };
struct realtek_priv { struct rtl8365mb *chip_data; void *map, *dev; };
static unsigned int rtl8367d_rd05_ext1_txc_tap;
static int step, fail, mismatch;
static bool enabled;
static u32 coarse, fine;
static bool rtl8367d_rd05_board(struct rtl8365mb *m) { return m->rd05 && m->family_d; }
static int next(void) { return ++step==fail ? -ENXIO : 0; }
static int rtl8367d_ext_config_forcemode(struct realtek_priv *p, int port,
 bool link, int speed, int duplex, bool tx, bool rx) {
 assert(port==7); assert(step==(link ? 5 : 0));
 if(link) { assert(speed==1000 && duplex==1 && tx && rx); }
 int ret=next(); if(!ret) enabled=link; return ret;
}
static int rtl8367d_ext1_config_rgmii(struct realtek_priv *p) {
 assert(step==1 && !enabled); int ret=next();
 if(!ret) fine=0x104 | FIELD_PREP(0x38,rtl8367d_rd05_ext1_txc_tap);
 return ret;
}
static int regmap_update_bits(void *m, u32 reg,u32 mask,u32 val) {
 assert(step==2 && !enabled && reg==0x1307 && mask==15);
 int ret=next(); if(!ret) coarse=(0x8000 | val); return ret;
}
static int regmap_read(void *m, u32 reg, u32 *val) {
 assert(!enabled); assert((step==3 && reg==0x1307) || (step==4 && reg==0x13f9));
 int ret=next(); if(!ret) *val=(reg==0x1307 ? coarse : fine) ^
 ((mismatch==1 && reg==0x1307) ? 1 : (mismatch==2 && reg==0x13f9) ? 8 : 0);
 return ret;
}
''' + helper + r'''
int main(void) {
 struct rtl8365mb mb={true,true}; struct realtek_priv p={.chip_data=&mb};
 for(int board=0;board<2;board++) {
  mb.rd05=board; mb.family_d=!board; step=0;
  assert(!rtl8367d_rd05_configure_link(&p,0,5) && !step);
 }
 mb.rd05=mb.family_d=true;
 for(int tx=0;tx<=1;tx++) for(int rx=0;rx<8;rx++) for(int tap=0;tap<8;tap++) {
  step=0; enabled=true; rtl8367d_rd05_ext1_txc_tap=tap;
  assert(!rtl8367d_rd05_configure_link(&p,tx,rx)); assert(step==6 && enabled);
  assert((coarse&15)==(tx*8+rx)); assert((fine&0x38)==(u32)(tap*8));
  assert((fine&~0x38u)==0x104);
 }
 step=0; assert(rtl8367d_rd05_configure_link(&p,2,5)==-EINVAL && !step);
 assert(rtl8367d_rd05_configure_link(&p,0,8)==-EINVAL && !step);
 for(fail=1;fail<=6;fail++) {
  step=0; enabled=true;
  assert(rtl8367d_rd05_configure_link(&p,0,5)==-ENXIO && step==fail);
  assert(enabled==(fail==1));
 }
 fail=0;
 for(mismatch=1;mismatch<=2;mismatch++) {
  step=0; enabled=true;
  assert(rtl8367d_rd05_configure_link(&p,0,5)==-EIO && !enabled);
  assert(step==(mismatch==1 ? 4 : 5));
 }
 return 0;
}
''')
            binary = Path(tmp) / 'test'
            subprocess.run(['cc', '-Wall', '-Werror', '-fsanitize=undefined',
                            str(p), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
