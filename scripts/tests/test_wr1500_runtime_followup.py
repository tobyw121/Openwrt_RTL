import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT,c_function
from test_r76_oem_port import run_c
G6=Path(os.environ.get('WR1500_G6_SOURCE','/nonexistent'))
class Followup(unittest.TestCase):
    def test_probe_frames(self):
        spec=importlib.util.spec_from_file_location('probe',ROOT/'scripts/diagnostics/r76-rx-probe.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        for length in m.LENGTHS:
            b=m.frame(m.mac('02:11:22:33:44:55'),m.mac('02:aa:bb:cc:dd:ee'),length,10)
            self.assertEqual(len(b),length);self.assertEqual(b[12:18],b'\x88\xb5R76P')
            self.assertEqual(int.from_bytes(b[18:20],'big'),length)
        with self.assertRaises(ValueError):m.mac('00:00:00:00:00:00')

    @unittest.skipUnless((G6/'phl/phl_types.h').exists(),'prepared source required')
    def test_phl_root_before_platform_header(self):
        cmd=['cpp','-dM','-DPHL_PLATFORM_LINUX','-DLINUX_VERSION_CODE=393330','-DKERNEL_VERSION(a,b,c)=((a<<16)|(b<<8)|c)','-DCONFIG_RTL8832BR','-DCONFIG_WLAN0_RFE_TYPE_50','-include',str(G6/'phl/phl_types.h'),'-']
        result=subprocess.run(cmd,input='',text=True,capture_output=True,check=True)
        self.assertIn('#define REALTEK_CONFIG_PATH "/etc/conf/"',result.stdout)

    @unittest.skipUnless((G6/'os_dep/linux/os_intfs.c').exists(),'prepared source required')
    def test_registration_mac_validation(self):
        fn=c_function(G6/'os_dep/linux/os_intfs.c','int rtw_os_ndev_register(')
        run_c(r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#define CONFIG_RTL8832BR 1
#define CONFIG_WLAN0_RFE_TYPE_50 1
#define KERNEL_VERSION(a,b,c) (((a)<<16)|((b)<<8)|(c))
#define LINUX_VERSION_CODE KERNEL_VERSION(6,6,114)
#define _SUCCESS 1
#define _FAIL 0
#define RTW_INFO(...) do{}while(0)
#define dev_warn(...) do{}while(0)
struct net_device{unsigned char addr[6];};struct dvobj_priv{int unused;};
typedef struct{struct net_device *pnetdev;struct dvobj_priv dvobj;unsigned char mac_addr[6];int registered;} _adapter;
#define adapter_to_dvobj(a) (&(a)->dvobj)
#define adapter_mac_addr(a) ((a)->mac_addr)
static int randoms;
static bool is_valid_ether_addr(const unsigned char *m){return !(m[0]&1) && memcmp(m,"\0\0\0\0\0\0",6);}
static void eth_random_addr(unsigned char *m){randoms++;memcpy(m,"\x02\x11\x22\x33\x44\x55",6);}
static void eth_hw_addr_set(struct net_device *n,const unsigned char *m){memcpy(n->addr,m,6);}
static void rtw_init_netdev_name(struct net_device *n,const char *s){(void)n;(void)s;}
static int rtw_linux_register_netdev(struct dvobj_priv *d,struct net_device *n){(void)d;assert(is_valid_ether_addr(n->addr));return 0;}
'''+fn+r'''
int main(void){struct net_device n;_adapter a={.pnetdev=&n};
 assert(rtw_os_ndev_register(&a,"wlan0")==1);assert(randoms==1 && a.registered);
 unsigned char saved[6];memcpy(saved,a.mac_addr,6);
 assert(rtw_os_ndev_register(&a,"wlan0")==1);assert(randoms==1);assert(!memcmp(saved,a.mac_addr,6));
 return 0;}
''')

    @unittest.skipUnless((G6/'os_dep/linux/rtw_cfg.c').exists(),'prepared source required')
    def test_nvm_less_rf_path_is_populated(self):
        import re
        fn=c_function(G6/'os_dep/linux/rtw_cfg.c','void rtw_load_phy_file_path (')
        names=set(re.findall(r'SET_PARA_INFO_EXT\((\w+)\)',fn)) - {'para_info_name'}
        flags=set(re.findall(r'rtw_load_phy_file & (\w+)',fn))
        code=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define CONFIG_LOAD_PHY_PARA_FROM_FILE 1
#define CONFIG_RTL8832BR 1
#define CONFIG_WLAN0_RFE_TYPE_50 1
#define RTW_PARA_SRC_EXTNAL 1
#define RTW_PRINT(...) do{}while(0)
struct para { int para_src; char para_path[256]; };
struct cap {
'''+''.join('struct para '+n+';\n' for n in names)+r'''
};
struct rtw_phl_com_t {struct cap phy_sw_cap[2];};
struct dvobj_priv {struct rtw_phl_com_t *phl;};
#define GET_HAL_DATA(d) ((d)->phl)
static char *rtw_phy_file_path="/etc/conf/";
static unsigned rtw_load_phy_file=0xffffffff;
'''+''.join('#define '+n+' 1\n' for n in flags)+fn+r'''
int main(void){struct rtw_phl_com_t phl={0};struct dvobj_priv d={.phl=&phl};
 rtw_load_phy_file_path(&d);
'''+''.join('assert(!strcmp(phl.phy_sw_cap[0].'+n+'.para_path,"/etc/conf/rtl8832bre/RFE50/"));\n' for n in names)+r'''
 return 0;}
'''
        run_c(code)
