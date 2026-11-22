"""Run the production TX cleaner with DMA/CDP and ownership fault injection."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function

DRIVER = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'

class R76TXLifetimeTests(unittest.TestCase):
    def test_production_cleaner(self):
        fn = c_function(DRIVER, 'static unsigned int rtl8197f_rtk_tx_clean(')
        counters = sorted(set(re.findall(r'priv->(mw5_tx_\w+|tx_bql_\w+|tx_clean_empty)', fn)))
        fields = '\n'.join('unsigned long long ' + n + ';' for n in counters)
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;
#define RTL_RTK_DESC_OWN 1
#define RTL_RTK_DESC_WRAP 2
#define READ_ONCE(x) (x)
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define unlikely(x) (x)
#define MAX_SKB_FRAGS 1
#define DMA_TO_DEVICE 0
#define spin_lock_irqsave(l,f) do { (void)(l); (f)=0; } while (0)
#define spin_unlock_irqrestore(l,f) do { (void)(l); (void)(f); } while (0)
#define dma_rmb() do {} while (0)
struct sk_buff { int unused; };
struct net_device { struct { unsigned tx_packets,tx_bytes; } stats; };
struct rtl8197f_rtk_desc { u32 opts1,addr,opts2,opts3,opts4,opts5; };
struct rtl8197f_rtknet_priv {
 struct net_device *ndev; bool is_mw5; void *dev; int tx_lock;
 unsigned tx_tail,tx_head,tx_ring_size;
 struct sk_buff *tx_skb[8]; unsigned tx_pkt_len[8],tx_dma[8],tx_len[8];
 bool tx_dma_is_page[8]; struct rtl8197f_rtk_desc desc[8];
''' + fields + r'''
};
static bool r76,valid; static u32 cdp; static int freed;
static bool rtl8197f_rtk_is_r76(void) { return r76; }
static bool rtl8197f_rtk_tx_cdp_index(struct rtl8197f_rtknet_priv *p,u32 *idx)
{ (void)p; *idx=cdp; return valid; }
static struct rtl8197f_rtk_desc *rtl8197f_rtk_tx_desc(struct rtl8197f_rtknet_priv *p,unsigned idx) { return &p->desc[idx]; }
static void rtl8197f_rtk_mw5_tx_complete_trace(struct rtl8197f_rtknet_priv *p,unsigned idx,u32 opts) { (void)p;(void)idx;(void)opts; }
#define dma_unmap_page(...) do {} while (0)
#define dma_unmap_single(...) do {} while (0)
static void dev_consume_skb_any(struct sk_buff *s) { (void)s;freed++; }
static void rtl8197f_rtk_desc_clear_padding(struct rtl8197f_rtknet_priv *p,struct rtl8197f_rtk_desc *d) { (void)p;(void)d; }
static unsigned rtl8197f_rtk_next(unsigned idx,unsigned size) { return (idx+1)%size; }
static bool netif_queue_stopped(struct net_device *n) { (void)n;return false; }
static unsigned rtl8197f_rtk_tx_avail(struct rtl8197f_rtknet_priv *p) { (void)p;return 7; }
static void netif_wake_queue(struct net_device *n) { (void)n; }
static void netdev_completed_queue(struct net_device *n,unsigned pkts,unsigned bytes) { (void)n;(void)pkts;(void)bytes; }
''' + fn + r'''
static struct net_device n;
static struct sk_buff skb;
static struct rtl8197f_rtknet_priv setup(unsigned tail,unsigned head) {
 struct rtl8197f_rtknet_priv p={.ndev=&n,.tx_ring_size=8,.tx_tail=tail,.tx_head=head};
 for(unsigned i=0;i<8;i++) { p.tx_skb[i]=&skb;p.tx_pkt_len[i]=100;p.desc[i].addr=0x81234560; }
 valid=true;freed=0;return p;
}
int main(void) {
 for(int board=0;board<3;board++) {
  r76=board==0;
  struct rtl8197f_rtknet_priv p=setup(0,2);p.is_mw5=board==1;cdp=0;
  assert(rtl8197f_rtk_tx_clean(&p)==(board==2 ? 2:0));
  p=setup(0,2);p.is_mw5=board==1;cdp=3; /* Beyond submitted interval. */
  assert(rtl8197f_rtk_tx_clean(&p)==(board==2 ? 2:0));
  p=setup(0,2);p.is_mw5=board==1;valid=false;
  assert(rtl8197f_rtk_tx_clean(&p)==(board==2 ? 2:0));
  p=setup(0,2);p.is_mw5=board==1;cdp=2;p.desc[0].opts1=RTL_RTK_DESC_OWN;
  assert(!rtl8197f_rtk_tx_clean(&p));assert(!freed);assert(p.tx_skb[0]);
  p=setup(6,1);p.is_mw5=board==1;cdp=0; /* Wrap, slot zero not complete. */
  assert(rtl8197f_rtk_tx_clean(&p)==(board==2 ? 3:2));
  assert(p.tx_tail==(board==2 ? 1:0));
  if(board!=2) assert(p.desc[6].addr==0x81234560); /* Preserve completed slot. */
  p=setup(0,2);p.is_mw5=board==1;cdp=2;
  assert(rtl8197f_rtk_tx_clean(&p)==2);assert(freed==2);assert(!p.tx_skb[0]);
 }
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'test.c';src.write_text(code)
            binary=Path(tmp)/'test'
            subprocess.run(['cc','-Wall','-Werror','-fsanitize=undefined',str(src),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

    def test_production_geometry_board_scope(self):
        text=DRIVER.read_text()
        start=text.index('#define RTL_RTK_TXRINGCR_TX_RING0_EN')
        end=text.index('\n\n',start)
        defs=text[start:end]
        fn=c_function(DRIVER,'static void rtl8197f_rtk_program_tx_ring_geometry(')
        code=r'''
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;
#define BIT(n) (1U<<(n))
#define RTL_RTK_CPUTPDCR0 0
#define RTL_RTK_DMA_CR1 1
#define RTL_RTK_DMA_CR4 2
#define RTL_RTK_TXRINGCR 3
#define RTL_RTK_DMA_CR4_TX_RING0_TAIL_AWARE 1
#define dev_warn(...) do {} while(0)
#define dev_info(...) do {} while(0)
#define wmb() do {} while(0)
static const char *board;
static u32 regs[4];
struct rtl8197f_rtknet_priv { u32 tx_desc_dma,tx_ring_size,desc_stride; bool sdk_swcore_init; unsigned txringcr_sdk_writes; };
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
static bool rtl8197f_rtk_is_r76(void) { return of_machine_is_compatible("cudy,ax1500-r76"); }
static u32 rtl8197f_rtk_hw_dma_addr(struct rtl8197f_rtknet_priv *p,u32 v) { (void)p;return v; }
static u32 rtl8197f_rtk_read(struct rtl8197f_rtknet_priv *p,unsigned reg) { (void)p;return regs[reg]; }
static void rtl8197f_rtk_write(struct rtl8197f_rtknet_priv *p,unsigned reg,u32 v) { (void)p;regs[reg]=v; }
''' + defs + '\n' + fn + r'''
int main(void) {
 const char *boards[]={"cudy,ax1500-r76","tenda,nova-mw5","xiaomi,r4-rd05"};
 for(int b=0;b<3;b++) {
  struct rtl8197f_rtknet_priv p={.tx_desc_dma=0xa3600000,.tx_ring_size=768,.desc_stride=32,.sdk_swcore_init=true};
  board=boards[b];regs[3]=0x80090f1f;
  rtl8197f_rtk_program_tx_ring_geometry(&p,"test");
  assert(regs[0]==p.tx_desc_dma);assert(regs[1]==767*32);
  assert(regs[3]==(b<2 ? 0x80090f11:0x80090f13));
 }
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'test.c';src.write_text(code);binary=Path(tmp)/'test'
            subprocess.run(['cc','-Wall','-Werror','-fsanitize=undefined',str(src),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)
