"""Exercise production G6 DMA and regulatory code, plus package payload integrity."""
from pathlib import Path
import os
import json
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'package/kernel/rtl8832br-wr1500'
PATCH = PACKAGE / 'patches/100-linux66-openwrt-wr1500.patch'


def new_fragment(file):
    out = []
    active = False
    for line in PATCH.read_text(errors='surrogateescape').splitlines():
        if line.startswith('+++ '):
            active = line == '+++ b/' + file
        elif active and line.startswith('--- '):
            active = False
        elif active and line.startswith(('+', ' ')):
            out.append(line[1:])
    return '\n'.join(out)


def function(text, name):
    start = text.index(name + '(')
    start = text.rfind('\n', 0, start) + 1
    brace = text.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        if text[end] == '{':
            depth += 1
        elif text[end] == '}':
            depth -= 1
        end += 1
    return text[start:end]


def compile_run(code):
    with tempfile.TemporaryDirectory(prefix='wr1500-g6-') as tmp:
        source, binary = Path(tmp) / 'check.c', Path(tmp) / 'check'
        source.write_text(code)
        result = subprocess.run(['cc', '-std=gnu11', '-Wall', '-Werror',
                                 '-fsanitize=undefined', str(source), '-o', str(binary)],
                                capture_output=True, text=True)
        if result.returncode:
            raise AssertionError(result.stderr)
        subprocess.run([str(binary)], check=True, capture_output=True)


class G6Tests(unittest.TestCase):
    def test_key_removal_reaches_hardware_and_reports_failure(self):
        fragment = function(new_fragment('os_dep/linux/ioctl_cfg80211.c'), 'cfg80211_rtw_del_key')
        code = '''
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
typedef unsigned char u8;
#define BIT(n) (1U<<(n))
#define WEP_KEYS 4
#define RTW_SEC_KEY_UNICAST 0
#define RTW_SEC_KEY_MULTICAST 1
#define PHL_CMD_DIRECTLY 0
#define _TRUE 1
#define _FALSE 0
struct wiphy {int unused;};
struct sta_info {bool ieee8021x_blocked,bpairwise_key_installed;u8 key_installed[2];char dot118021x_UncstKey[32];};
struct security_priv {char dot118021XGrpKey[4][32];u8 dot118021XGrpKeyid,key_mask;bool binstallGrpkey;};
typedef struct {struct security_priv securitypriv;int stapriv,mlmepriv;} _adapter;
struct net_device {_adapter *adapter;};
static struct sta_info sta;
static int hw_fail,hw_calls,is_ap=1,missing;
#define rtw_netdev_priv(n) ((n)->adapter)
#define MLME_IS_AP(a) (is_ap)
#define get_bssid(m) ((const u8 *)"123456")
static struct sta_info *rtw_get_stainfo(void *p,const u8 *m)
{(void)p;(void)m;return missing?NULL:&sta;}
static struct sta_info *rtw_get_bcmc_stainfo(_adapter *a)
{(void)a;return missing?NULL:&sta;}
static int rtw_hw_del_key(_adapter *a,struct sta_info *s,u8 key,u8 type,int cmd,int timeout)
{(void)a;(void)cmd;(void)timeout;hw_calls++;if(hw_fail)return -1;s->key_installed[type]&=~BIT(key);return 0;}
static void memzero_explicit(void *p,size_t n) {memset(p,0,n);}
'''
        code += fragment
        code += '''
int main(void) {
 _adapter a={0};struct net_device n={&a};const u8 peer[6]={1};
 memset(sta.dot118021x_UncstKey,0x5a,32);sta.bpairwise_key_installed=true;
 sta.key_installed[0]=1;hw_fail=1;
 assert(cfg80211_rtw_del_key(NULL,&n,0,0,true,peer)==-EIO);
 assert(hw_calls==1 && sta.ieee8021x_blocked && !sta.bpairwise_key_installed);
 assert(sta.dot118021x_UncstKey[0]==0x5a);
 hw_fail=0;assert(cfg80211_rtw_del_key(NULL,&n,0,0,true,peer)==0);
 assert(hw_calls==2 && !sta.key_installed[0] && sta.dot118021x_UncstKey[0]==0);
 assert(cfg80211_rtw_del_key(NULL,&n,0,0,true,peer)==0 && hw_calls==2);
 a.securitypriv.binstallGrpkey=true;a.securitypriv.dot118021XGrpKeyid=1;
 memset(a.securitypriv.dot118021XGrpKey[1],0x5a,32);sta.key_installed[1]=BIT(1);
 assert(cfg80211_rtw_del_key(NULL,&n,0,1,false,NULL)==0 && hw_calls==3);
 assert(!a.securitypriv.binstallGrpkey && !a.securitypriv.dot118021XGrpKey[1][0]);
 assert(cfg80211_rtw_del_key(NULL,&n,1,0,true,peer)==-EOPNOTSUPP);
 assert(cfg80211_rtw_del_key(NULL,&n,0,4,false,NULL)==-EOPNOTSUPP);
 assert(cfg80211_rtw_del_key(NULL,&n,0,0,true,NULL)==-EINVAL);
 missing=1;assert(cfg80211_rtw_del_key(NULL,&n,0,0,true,peer)==0 && hw_calls==3);
 return 0;
}
'''
        compile_run(code)

    @unittest.skipUnless(os.environ.get('WR1500_G6_SOURCE'), 'requires prepared GPL source tree')
    def test_tx_partial_mapping_unwinds_without_double_unmap(self):
        source = (Path(os.environ['WR1500_G6_SOURCE'])/'core/rtw_xmit.c').read_text()
        code = '''
#include <assert.h>
#include <stdbool.h>
#include <string.h>
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long dma_addr_t;
struct pci_dev { int unused; };
struct pci_data { struct pci_dev *ppcidev; };
typedef struct pci_data *PPCI_DATA;
typedef struct { struct pci_data *dvobj; } _adapter;
struct rtw_pkt_buf_list { u8 dma_mapped; void *vir_addr; u32 phy_addr_l,phy_addr_h; unsigned short length; };
struct rtw_xmit_req { u8 *pkt_list; u32 pkt_cnt; };
struct xmit_frame { struct rtw_xmit_req *phl_txreq; u32 txreq_cnt; };
#define dvobj_to_pci(d) (d)
#define XF_TXREQ (pxframe->phl_txreq)
#define XF_TXREQ_CNT (pxframe->txreq_cnt)
#define PHLTX_ENTER do {} while (0)
#define DMA_TO_DEVICE 1
#define _SUCCESS 1
#define _FAIL 0
static int maps,unmaps,fail_at;
static bool active[8];
static int pci_get_bus_addr(struct pci_dev *p,void *v,dma_addr_t *a,unsigned int n,int d)
{ (void)p;(void)v;(void)n;assert(d==DMA_TO_DEVICE);maps++;if(maps==fail_at)return -1;
 *a=maps-1;active[*a]=true;return 0; }
static void pci_unmap_bus_addr(struct pci_dev *p,dma_addr_t *a,unsigned int n,int d)
{ (void)p;(void)n;assert(d==DMA_TO_DEVICE && active[*a]);active[*a]=false;unmaps++; }
'''
        code += function(source, 'core_recycle_txreq_phyaddr')
        code += function(source, 'fill_txreq_phyaddr')
        code += '''
int main(void) {
 struct pci_dev dev={0};struct pci_data pci={&dev};_adapter adapter={&pci};
 struct rtw_pkt_buf_list packets[3][2];struct rtw_xmit_req req[3];
 struct xmit_frame frame={req,3};int fault,i,j,result;
 for(fault=0;fault<=6;fault++) {
  maps=unmaps=0;fail_at=fault;memset(active,0,sizeof(active));
  memset(packets,0xa5,sizeof(packets));
  for(i=0;i<3;i++) {req[i].pkt_list=(u8 *)packets[i];req[i].pkt_cnt=2;
   for(j=0;j<2;j++){packets[i][j].vir_addr=&dev;packets[i][j].length=16;}}
  result=fill_txreq_phyaddr(&adapter,&frame);
  assert(result==(fault?_FAIL:_SUCCESS));
  if(fault)assert(unmaps==fault-1);else assert(unmaps==0 && maps==6);
  for(i=0;i<3;i++)core_recycle_txreq_phyaddr(&adapter,&req[i]);
  assert(unmaps==(fault?fault-1:6));
  for(i=0;i<8;i++)assert(!active[i]);
 }
 return 0;
}
'''
        compile_run(code)

    def test_production_dma_mapping_failures_and_zero_address(self):
        fragment = new_fragment('platform/platform_mips_97f_pci.c')
        names = ['pci_get_bus_addr', 'pci_unmap_bus_addr', 'pci_alloc_cache_mem',
                 'pci_free_cache_mem', 'pci_alloc_noncache_mem', 'pci_free_noncache_mem']
        code = '''
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
typedef unsigned long dma_addr_t;
struct device { int unused; };
struct pci_dev { struct device dev; };
#define GFP_KERNEL 0
static int map_fail, maps, unmaps, frees, alloc_fail, coherent_frees;
static void *rtw_zmalloc(size_t n) { return alloc_fail ? NULL : calloc(1,n); }
static void rtw_mfree(void *p, size_t n) { (void)n; frees++; free(p); }
static dma_addr_t dma_map_single(struct device *d,void *p,size_t n,int dir)
{ (void)d;(void)p;(void)n;(void)dir; maps++; return 0; }
static int dma_mapping_error(struct device *d,dma_addr_t a)
{ (void)d;(void)a; return map_fail; }
static void dma_unmap_single(struct device *d,dma_addr_t a,size_t n,int dir)
{ (void)d;(void)a;(void)n;(void)dir;unmaps++; }
static void *dma_alloc_coherent(struct device *d,size_t n,dma_addr_t *a,int flags)
{ (void)d;(void)flags;*a=0;return alloc_fail ? NULL : calloc(1,n); }
static void dma_free_coherent(struct device *d,size_t n,void *p,dma_addr_t a)
{ (void)d;(void)n;(void)a;coherent_frees++;free(p); }
'''
        code += '\n'.join(function(fragment, name) for name in names)
        code += '''
int main(void) {
 struct pci_dev p = {0}; dma_addr_t a=42; char data[16]; void *buf;
 assert(pci_get_bus_addr(NULL,data,&a,16,1)==-EINVAL && maps==0);
 assert(pci_get_bus_addr(&p,NULL,&a,16,1)==-EINVAL && maps==0);
 assert(pci_get_bus_addr(&p,data,&a,0,1)==-EINVAL && maps==0);
 assert(pci_get_bus_addr(&p,data,&a,16,1)==0 && a==0 && maps==1);
 map_fail=1;
 assert(pci_get_bus_addr(&p,data,&a,16,1)==-EIO);
 assert(pci_alloc_cache_mem(&p,&a,16,1)==NULL && frees==1 && unmaps==0);
 alloc_fail=1; assert(pci_alloc_cache_mem(&p,&a,16,1)==NULL && frees==1);
 alloc_fail=0;map_fail=0;
 buf=pci_alloc_cache_mem(&p,&a,16,1);assert(buf && a==0);
 pci_free_cache_mem(&p,buf,&a,16,1);assert(unmaps==1 && frees==2);
 assert(pci_alloc_noncache_mem(NULL,&a,16)==NULL);
 buf=pci_alloc_noncache_mem(&p,&a,16);assert(buf && a==0);
 pci_free_noncache_mem(&p,buf,&a,16);assert(coherent_frees==1);
 return 0;
}
'''
        compile_run(code)

    def test_regulatory_intersection_never_widens_core_restrictions(self):
        fragment = function(new_fragment('os_dep/linux/wifi_regd.c'), 'rtw_regd_apply_flags')
        code = '''
#include <assert.h>
#include <stdbool.h>
#define NUM_NL80211_BANDS 2
#define IEEE80211_CHAN_DISABLED 1
#define IEEE80211_CHAN_NO_IR 2
#define IEEE80211_CHAN_RADAR 4
#define RTW_CHF_DFS 1
#define RTW_CHF_NO_IR 2
typedef struct { unsigned int ChannelNum, flags; } RT_CHANNEL_INFO;
struct rf_ctl_t { unsigned int max_chan_nums; RT_CHANNEL_INFO channel_set[4]; };
struct ieee80211_channel { unsigned int hw_value, flags; };
struct ieee80211_supported_band { unsigned int n_channels; struct ieee80211_channel *channels; };
struct wiphy { struct ieee80211_supported_band *bands[2]; struct rf_ctl_t *rf; };
#define wiphy_to_dvobj(w) ((w)->rf)
#define dvobj_to_rfctl(p) (p)
'''
        code += fragment
        code += '''
int main(void) {
 struct rf_ctl_t rf={3,{{36,0},{40,RTW_CHF_DFS},{44,RTW_CHF_NO_IR}}};
 struct ieee80211_channel ch[]={{36,IEEE80211_CHAN_NO_IR}, {40,0},
  {44,IEEE80211_CHAN_RADAR}, {48,0}, {52,IEEE80211_CHAN_DISABLED}};
 struct ieee80211_supported_band band={5,ch};struct wiphy w={{0,&band},&rf};
 rtw_regd_apply_flags(&w);
 assert(ch[0].flags==IEEE80211_CHAN_NO_IR);
 assert(ch[1].flags==IEEE80211_CHAN_RADAR);
 assert(ch[2].flags==(IEEE80211_CHAN_RADAR|IEEE80211_CHAN_NO_IR));
 assert(ch[3].flags==IEEE80211_CHAN_DISABLED && ch[4].flags==IEEE80211_CHAN_DISABLED);
 return 0;
}
'''
        compile_run(code)

    def test_installed_rfe_payload_matches_oem_manifest(self):
        import hashlib
        manifest = json.loads((ROOT/'reverse-engineering/wr1500-wifi6/oem-rfe50-comparison.json').read_text())
        expected = {x['path']: x['oem_sha256'] for x in manifest['files']
                    if '/' not in x['path'] or x['path'].startswith('AP_5G/')}
        files = {str(p.relative_to(PACKAGE/'files/RFE50')): p
                 for p in (PACKAGE/'files/RFE50').rglob('*.txt')}
        self.assertEqual(set(files), set(expected))
        for name, path in files.items():
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), expected[name], name)


if __name__ == '__main__':
    unittest.main()
