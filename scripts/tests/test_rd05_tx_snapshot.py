"""Bounded diagnostic quota, including non-RD05 isolation and oversized arms."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function


class RD05TXSnapshotTests(unittest.TestCase):
    def test_coherent_policy_latches_only_rd05_and_preserves_other_boards(self):
        source = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
        helper = c_function(source, 'static bool rtl8197f_rtk_rd05_tx_coherent_policy(')
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'test.c'
            p.write_text("""
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#define READ_ONCE(x) (x)
static bool rd05_tx_coherent;
static const char *board;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
""" + helper + """
int main(void) {
 const char *boards[]={"xiaomi,r4-rd05","tenda,nova-mw5","tenda,ac23"};
 for(int b=0;b<3;b++) for(int mw=0;mw<2;mw++)
 for(int old=0;old<2;old++) for(int requested=0;requested<2;requested++) {
  board=boards[b]; rd05_tx_coherent=requested;
  bool active=rtl8197f_rtk_rd05_tx_coherent_policy(mw,old);
  assert(active==((b==0 && !mw) ? requested : old));
  rd05_tx_coherent=!requested; /* running allocation remains latched */
  assert(active==((b==0 && !mw) ? requested : old));
 }
 return 0;
}
""")
            exe = Path(tmp) / 'test'
            subprocess.run(['cc','-Wall','-Werror',str(p),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)

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
