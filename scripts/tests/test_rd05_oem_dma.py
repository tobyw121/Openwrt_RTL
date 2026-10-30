#!/usr/bin/env python3
"""Execute the RD05-only OEM address policy and TX metadata against board mocks."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function

DRIVER = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'


class RD05OEMDMATests(unittest.TestCase):
    def test_external_switch_policy_requires_rd05_and_d_family(self):
        patch = ROOT / 'target/linux/realtek/patches-6.6/337-net-dsa-realtek-scope-rd05-cpu-policy.patch'
        added = '\n'.join(line[1:] for line in patch.read_text().splitlines()
                          if line.startswith('+') and not line.startswith('+++'))
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'added.c'
            source.write_text(added)
            helper = c_function(source, 'static bool rtl8367d_rd05_board(')
            source.write_text(r'''
#include <assert.h>
#include <stdbool.h>
#include <string.h>
static const char *board;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
struct info { bool rtl8367d; };
struct rtl8365mb { struct info *chip_info; };
''' + helper + r'''
int main(void) {
 const char *boards[]={"xiaomi,r4-rd05","tenda,nova-mw5","tenda,ac23","iball,wrd12gn"};
 struct info info; struct rtl8365mb mb={.chip_info=&info};
 for(int b=0;b<4;b++) for(int d=0;d<2;d++) {
  board=boards[b]; info.rtl8367d=d;
  assert(rtl8367d_rd05_board(&mb)==(b==0 && d));
 }
 board=boards[0]; mb.chip_info=0;
 assert(!rtl8367d_rd05_board(&mb));
 return 0;
}
''')
            binary = Path(tmp) / 'test'
            subprocess.run(['cc', '-Wall', '-Werror', '-fsanitize=undefined',
                            str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_fifo_watermarks_match_rd05_oem_and_preserve_other_boards(self):
        definitions = '\n'.join(line for line in DRIVER.read_text().splitlines()
                                if line.startswith('#define RTL_RTK_DMA_CR0_'))
        code = r'''
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;
static const char *board;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
''' + definitions + '\n' + c_function(DRIVER, 'static u32 rtl8197f_rtk_dma_fifo_watermarks(') + r'''
int main(void) {
 const char *boards[]={"xiaomi,r4-rd05","tenda,nova-mw5","tenda,ac23","iball,wrd12gn","generic"};
 const u32 values[]={0,0xffffffff,0x000330d7,0x0003a0ce,0xa5a51234};
 for(int b=0;b<5;b++) for(int i=0;i<5;i++) {
  board=boards[b];
  u32 result=rtl8197f_rtk_dma_fifo_watermarks(values[i]);
  assert(result==((values[i]&0xffff0000)|(b==0 ? 0xa0ce : 0x30d7)));
 }
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'test.c'
            binary = Path(tmp) / 'test'
            source.write_text(code)
            subprocess.run(['cc', '-Wall', '-Werror', '-fsanitize=undefined',
                            str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_board_scope_dma_handles_ring_addresses_and_tx_metadata(self):
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint64_t dma_addr_t;
#define BIT(n) (1U << (n))
#define READ_ONCE(x) (x)
#define lower_32_bits(x) ((u32)(x))
#define RTL_RTK_TX_DP_MASK 0x7f
#define RTL_RTK_TX_DP_EXT_MASK 7
#define RTL_RTK_TX_EXTSPA_MASK 3
static const char *board;
static bool rd05_oem_buffer_alias;
static int rd05_txdesc_mode;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
struct rtl8197f_rtknet_priv {
 bool is_mw5, desc_addr_kseg1, desc_addr_physical, rd05_oem_buffer_alias;
 u32 tx_port_mask, tx_dp_ext, tx_extspa;
 bool tx_hwlookup, tx_bridge;
};
struct rtl8197f_rtk_txmeta {
 u32 port_mask, dp_ext, extspa;
 u16 vid;
 bool vid_valid, hwlookup, bridge;
};
''' + '\n'.join(c_function(DRIVER, name) for name in [
            'static u32 rtl8197f_rtk_hw_dma_addr(',
            'static bool rtl8197f_rtk_rd05_oem_buffers(',
            'static u32 rtl8197f_rtk_buffer_dma_addr(',
            'static void rtl8197f_rtk_rd05_txdesc_meta(']) + r'''
int main(void) {
 const char *boards[] = {"xiaomi,r4-rd05", "tenda,nova-mw5", "tenda,ac23", "iball,wrd12gn"};
 const dma_addr_t addresses[] = {0x01234560,0x81234560,0xa1234560,0x1fffffff};
 for (int b=0;b<4;b++) for (int enabled=0;enabled<2;enabled++) {
  board=boards[b]; rd05_oem_buffer_alias=enabled;
  struct rtl8197f_rtknet_priv p = {.is_mw5=b==1, .desc_addr_kseg1=true, .tx_port_mask=1};
  p.rd05_oem_buffer_alias=rtl8197f_rtk_rd05_oem_buffers();
  assert(p.rd05_oem_buffer_alias == (b==0 && enabled));
  for (int physical=0;physical<2;physical++) for (int kseg1=0;kseg1<2;kseg1++) {
   p.desc_addr_physical=physical; p.desc_addr_kseg1=kseg1;
   for (int i=0;i<4;i++) {
    dma_addr_t handle=addresses[i];
    u32 legacy=physical ? (u32)handle : kseg1 ? ((u32)handle & 0x1fffffff)|0xa0000000 : (u32)handle;
    u32 packet=rtl8197f_rtk_buffer_dma_addr(&p,handle);
    assert(rtl8197f_rtk_hw_dma_addr(&p,handle)==legacy);
    assert(packet == (p.rd05_oem_buffer_alias ? ((u32)handle & 0x1fffffff)|0x80000000 : legacy));
    assert(handle==addresses[i]); /* Original DMA handle retained for sync/unmap. */
   }
  }
  for (int mode=0;mode<=13;mode++) {
   struct rtl8197f_rtk_txmeta m;
   rd05_txdesc_mode=mode; rtl8197f_rtk_rd05_txdesc_meta(&p,&m);
   if (b!=0 || mode==0) assert(m.port_mask==1 && !m.hwlookup && !m.bridge && !m.vid_valid);
   if (b==0 && mode==13) assert(m.port_mask==2 && !m.dp_ext && !m.extspa && !m.hwlookup && !m.bridge && !m.vid_valid);
  }
 }
 /* Defense in depth even if a MW5 private structure has an erroneous flag. */
 struct rtl8197f_rtknet_priv mw5={.is_mw5=true,.rd05_oem_buffer_alias=true,.desc_addr_kseg1=true};
 assert(rtl8197f_rtk_buffer_dma_addr(&mw5,0x1234)==0xa0001234);
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'
            binary = Path(tmp) / 'test'
            c.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Wall', '-Werror',
                            '-fsanitize=undefined', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
