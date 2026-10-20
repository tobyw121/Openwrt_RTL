"""Match integrated HAL selection to archived OEM kernels and test failure exits."""
import importlib.util
import lzma
import subprocess
import tempfile
import unittest
from pathlib import Path

from test_mw5_stability import ROOT, WLAN, c_function


class IntegratedHalTests(unittest.TestCase):
    def test_package_hal_matches_each_boards_own_oem_kernel(self):
        spec = importlib.util.spec_from_file_location(
            'build_check', ROOT / 'scripts/check-rtl8192cd-linux6-build.py')
        build = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build)
        for board in ('rd05', 'ac23', 'mw5'):
            with self.subTest(board=board):
                symbols = lzma.open(ROOT / 'reverse-engineering/oem-kernels' /
                                    board / 'symbols.tsv.xz', 'rt').read()
                families = [family for family in ('8197F', '8197G')
                            if '\thal_Associate_' + family + '\n' in symbols]
                self.assertEqual(len(families), 1)
                flags = build.package_flags(board)
                selected = [family for family in ('8197F', '8197G')
                            if 'CONFIG_WLAN_HAL_' + family + '=y' in flags]
                self.assertEqual(selected, families)

    def test_failed_association_never_calls_uninitialized_hal_callbacks(self):
        function = c_function(WLAN / '8192cd_osdep.c', 'BOOLEAN\nWlan_HAL_Link(')
        program = r'''
#include <assert.h>
#include <stdbool.h>
typedef bool BOOLEAN;
typedef unsigned char *pu1Byte;
#define TRUE true
#define FALSE false
#define RT_STATUS_SUCCESS 0
#define HW_VAR_MAC_IO_ENABLE 1
#define HW_VAR_DRV_DBG 2
#define DRV_ER_CLOSE_STOP_HW 4
#define SYS_FUNC_EN 2
#define BIT2 4
#define DEBUG_INFO(...) ((void)0)
#define panic_printk(...) ((void)0)
struct rtl8192cd_priv { int unused; };
static int callbacks;
static int result;
static int mac_ready;
static int HalAssociateNic(struct rtl8192cd_priv *p, bool root) {
    (void)p; (void)root; return result;
}
static int get_mimo(struct rtl8192cd_priv *p) { (void)p; callbacks++; return 2; }
static void get_reg(struct rtl8192cd_priv *p, int reg, pu1Byte value) {
    (void)p; (void)reg; callbacks++; *value=mac_ready;
}
static int reset(struct rtl8192cd_priv *p) { (void)p; callbacks++; return 0; }
static void set_reg(struct rtl8192cd_priv *p, int reg, pu1Byte value) {
    (void)p; (void)reg; (void)value; callbacks++;
}
static struct { int MIMO_TR_hw_support; } hw;
static struct {
 int (*GetChipIDMIMOHandler)(struct rtl8192cd_priv *);
 void (*GetHwRegHandler)(struct rtl8192cd_priv *,int,pu1Byte);
 int (*ResetHWForSurpriseHandler)(struct rtl8192cd_priv *);
 int (*StopHWHandler)(struct rtl8192cd_priv *);
 void (*SetHwRegHandler)(struct rtl8192cd_priv *,int,pu1Byte);
} hal = {get_mimo,get_reg,reset,reset,set_reg};
#define GET_HW(p) (&hw)
#define GET_HAL_INTERFACE(p) (&hal)
#define RTL_R8(reg) 0
static void check_chipID_MIMO(struct rtl8192cd_priv *p) { (void)p; callbacks++; }
'''
        program += function + r'''
int main(void) {
 struct rtl8192cd_priv p={0};
 result=-1;
 assert(!Wlan_HAL_Link(&p));
 assert(callbacks==0);
 result=0;
 assert(Wlan_HAL_Link(&p));
 assert(callbacks==3);
 callbacks=0; mac_ready=1;
 hal.ResetHWForSurpriseHandler=0;
 assert(Wlan_HAL_Link(&p));
 assert(callbacks==4); /* MIMO, MAC state, mandatory stop, chip ID. */
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'hal.c'
            binary = Path(directory) / 'hal'
            source.write_text(program)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            str(source), '-o', str(binary)], check=True,
                           capture_output=True, text=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
