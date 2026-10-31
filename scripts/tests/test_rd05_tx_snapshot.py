"""Bounded diagnostic quota, including non-RD05 isolation and oversized arms."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function


class RD05TXSnapshotTests(unittest.TestCase):
    def test_quota_and_board_gate(self):
        source = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
        helper = c_function(source, 'static bool rtl8197f_rtk_rd05_snapshot_claim(')
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'test.c'
            p.write_text(r'''
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#define READ_ONCE(x) (x)
#define min_t(t,a,b) ((t)(a)<(t)(b)?(t)(a):(t)(b))
#define cmpxchg(p,a,b) __sync_val_compare_and_swap(p,a,b)
static unsigned int rd05_tx_snapshot_budget;
''' + helper + r'''
int main(void) {
 unsigned int arms[]={0,1,8,9,UINT_MAX};
 for(unsigned int i=0;i<5;i++) {
  rd05_tx_snapshot_budget=arms[i];
  assert(!rtl8197f_rtk_rd05_snapshot_claim(false));
  assert(rd05_tx_snapshot_budget==arms[i]);
  unsigned int count=0;
  while(rtl8197f_rtk_rd05_snapshot_claim(true)) { assert(++count<=8); }
  assert(count==(arms[i]>8 ? 8 : arms[i]));
  assert(!rd05_tx_snapshot_budget);
 }
 return 0;
}
''')
            exe = Path(tmp) / 'test'
            subprocess.run(['cc', '-Wall', '-Werror', '-fsanitize=undefined',str(p),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)
