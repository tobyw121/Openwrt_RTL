"""Compile production R76/G6 changes with host mocks; no radio or MMIO access."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT,c_function
DRIVER=ROOT/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
G6=Path(os.environ.get('WR1500_G6_SOURCE','/nonexistent'))

def run_c(code):
    with tempfile.TemporaryDirectory() as d:
        p=Path(d);(p/'test.c').write_text(code)
        subprocess.run(['cc','-Wall','-Werror','-fsanitize=undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
        subprocess.run([str(p/'test')],check=True)

class R76PortTests(unittest.TestCase):
    def test_actual_cdp_alias_and_stable_snapshot(self):
        code=r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint32_t u32;
#define RTL_RTK_DESC_OWN 1U
#define READ_ONCE(x) (x)
#define le32_to_cpu(x) (x)
struct rtl8197f_rtk_desc {u32 opts1,addr,opts2,opts3,opts4,opts5;};
struct rtl8197f_rtknet_priv {bool is_mw5;u32 tx_desc_dma,tx_ring_size,desc_stride;unsigned rx_desc_stable_retry,rx_desc_unstable;};
static bool r76;
static bool rtl8197f_rtk_is_r76(void){return r76;}
static u32 rtl8197f_rtk_hw_dma_addr(struct rtl8197f_rtknet_priv *p,u32 dma){(void)p;return dma|0xa0000000;}
static struct rtl8197f_rtk_desc *active;
static int barriers;
static bool tear;
static void dma_rmb(void){if(tear && ++barriers%2==0) active->opts4++;}
static void udelay(int n){(void)n;}
'''+c_function(DRIVER,'static bool rtl8197f_rtk_tx_cdp_index_value(')+c_function(DRIVER,'static bool rtl8197f_rtk_r76_rx_desc_stable(')+r'''
int main(void){
 struct rtl8197f_rtknet_priv p={.tx_desc_dma=0x3600000,.tx_ring_size=768,.desc_stride=32};
 u32 idx=999;
 r76=true;
 const u32 aliases[]={0x3605120,0x83605120,0xa3605120};
 for(int i=0;i<3;i++){assert(rtl8197f_rtk_tx_cdp_index_value(&p,aliases[i],&idx));assert(idx==649);}
 assert(!rtl8197f_rtk_tx_cdp_index_value(&p,0xa3605121,&idx));
 assert(!rtl8197f_rtk_tx_cdp_index_value(&p,0xa3606000,&idx));
 p.is_mw5=true;assert(!rtl8197f_rtk_tx_cdp_index_value(&p,0x83605120,&idx));
 assert(rtl8197f_rtk_tx_cdp_index_value(&p,0xa3605120,&idx));
 p.is_mw5=false;p.desc_stride=0;assert(!rtl8197f_rtk_tx_cdp_index_value(&p,aliases[0],&idx));
 struct rtl8197f_rtk_desc d={.opts1=0x06a4000c,.addr=0x87048862,.opts2=0x01800046};
 u32 o1=0,o2=0;active=&d;
 assert(rtl8197f_rtk_r76_rx_desc_stable(&p,&d,&o1,&o2));assert(o2==0x01800046);
 d.opts1|=1;assert(!rtl8197f_rtk_r76_rx_desc_stable(&p,&d,&o1,&o2));
 assert(p.rx_desc_unstable==1 && p.rx_desc_stable_retry==4);
 d.opts1&=~1U;tear=true;barriers=0;
 assert(!rtl8197f_rtk_r76_rx_desc_stable(&p,&d,&o1,&o2));assert(p.rx_desc_unstable==2);
 return 0;
}
'''
        run_c(code)

    @unittest.skipUnless((G6/'os_dep/linux/os_intfs.c').exists(),'set WR1500_G6_SOURCE to prepared source')
    def test_actual_cfg80211_lock_context(self):
        file=G6/'os_dep/linux/os_intfs.c'
        funcs=c_function(file,'int rtw_linux_register_netdev(')+c_function(file,'void rtw_linux_unregister_netdev(')
        run_c(r'''
#include <stdbool.h>
#include <assert.h>
#define CONFIG_IOCTL_CFG80211 1
#define KERNEL_VERSION(a,b,c) (((a)<<16)|((b)<<8)|(c))
#define LINUX_VERSION_CODE KERNEL_VERSION(6,6,114)
#define READ_ONCE(x) (x)
static int task,other;
#define current (&task)
struct dvobj_priv {void *cfg80211_lock_holder;void *rtnl_lock_holder;};
struct net_device {int unused;};
static int registered,unregistered,result;
static bool rtw_rtnl_lock_needed(struct dvobj_priv *d){return d->rtnl_lock_holder!=current;}
static int cfg80211_register_netdevice(struct net_device *n){(void)n;registered=1;return result;}
static int register_netdev(struct net_device *n){(void)n;registered=2;return result;}
static int register_netdevice(struct net_device *n){(void)n;registered=3;return result;}
static void cfg80211_unregister_netdevice(struct net_device *n){(void)n;unregistered=1;}
static void unregister_netdev(struct net_device *n){(void)n;unregistered=2;}
static void unregister_netdevice(struct net_device *n){(void)n;unregistered=3;}
'''+funcs+r'''
int main(void){
 struct dvobj_priv d={0};struct net_device n;
 for(int ctx=0;ctx<4;ctx++) for(int failure=0;failure<2;failure++){
  d.rtnl_lock_holder=ctx==1||ctx==2?current:0;
  d.cfg80211_lock_holder=ctx==2?current:ctx==3?&other:0;
  result=failure?-17:0;
  assert(rtw_linux_register_netdev(&d,&n)==result);rtw_linux_unregister_netdev(&d,&n);
  int expected=ctx==2?1:ctx==1?3:2;assert(registered==expected && unregistered==expected);
 }
 return 0;
}
''')
        cfg=(G6/'os_dep/linux/ioctl_cfg80211.c').read_text()
        self.assertEqual(cfg.count('WRITE_ONCE(dvobj->cfg80211_lock_holder, current);'),2)
        self.assertEqual(cfg.count('WRITE_ONCE(dvobj->cfg80211_lock_holder, NULL);'),2)
        self.assertNotIn('ret = register_netdevice(mon_ndev);',cfg)

    @unittest.skipUnless((G6/'os_dep/linux/os_intfs.c').exists(),'set WR1500_G6_SOURCE to prepared source')
    def test_actual_checksum_unaligned_and_oem_lengths(self):
        file=G6/'phl/hal_g6/mac/mac_ax/mac_8192xb/trx_desc_8192xb.c'
        funcs=c_function(file,'u32 mac_txdesc_len_8192xb(')+c_function(file,'u32 mac_wd_checksum_8192xb(')
        run_c(r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;
#define PLTFM_MSG_ERR(...) do{}while(0)
#define PLTFM_MSG_TRACE(...) do{}while(0)
#define MACNPTR 5
#define MACSUCCESS 0
#define RXD_SHORT_LEN 16
#define RXD_LONG_LEN 32
#define WD_BODY_LEN_V1 32
#define WD_INFO_LEN 24
#define MAC_AX_INTF_USB 0
enum rtw_packet_type{RTW_PHL_PKT_TYPE_H2C=2,RTW_PHL_PKT_TYPE_FWDL=4};
struct hw{int intf,wd_checksum_en;};
struct mac_ax_adapter{struct hw *hw_info;};
struct rtw_t_meta_data{enum rtw_packet_type type;u32 pktlen;int wdinfo_en;};
static u16 get_unaligned_le16(const u8 *p){return p[0]|((u16)p[1]<<8);}
static u32 get_unaligned_le32(const u8 *p){return get_unaligned_le16(p)|((u32)get_unaligned_le16(p+2)<<16);}
static void put_unaligned_le32(u32 v,u8 *p){for(int i=0;i<4;i++)p[i]=v>>(i*8);}
'''+funcs+r'''
int main(void){
 struct hw hw={.intf=1};struct mac_ax_adapter a={.hw_info=&hw};
 struct rtw_t_meta_data m={0};u8 storage[64],expected[64];
 assert(mac_wd_checksum_8192xb(&a,&m,0)==5);
 for(unsigned v=0;v<2048;v++){
  hw.intf=v%2;m.type=(v%3==0)?2:(v%3==1)?4:0;m.pktlen=v;m.wdinfo_en=v&1;
  unsigned len=(m.type==2||m.type==4)?((!hw.intf && ((v+16)&511)==0)?32:16):(m.wdinfo_en?56:32);
  assert(mac_txdesc_len_8192xb(&a,&m)==len);
  for(unsigned i=0;i<64;i++)storage[i]=expected[i]=(u8)(v*13+i*7);
  u8 *d=expected+1;d[16]=d[17]=0;u16 sum=0;
  for(unsigned i=0;i<len;i+=2)sum^=(u16)(d[i]|((u16)d[i+1]<<8));
  d[16]=sum;d[17]=sum>>8;
  assert(mac_wd_checksum_8192xb(&a,&m,storage+1)==0);
  assert(!memcmp(storage,expected,64));
 }
 return 0;
}
''')
