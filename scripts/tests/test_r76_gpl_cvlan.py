"""Test GPL-confirmed CVLAN readback using production register functions."""
from pathlib import Path
import unittest
from test_wr1500_wifi6 import function, compile_run
ROOT=Path(__file__).resolve().parents[2]
class GPLCVLAN(unittest.TestCase):
 def test_readback_and_error_paths(self):
  patch=next((ROOT/'target/linux/realtek/patches-6.6').glob('343-*')).read_text()
  code='\n'.join(l[1:] for l in patch.splitlines() if l.startswith('+') and not l.startswith('+++'))
  funcs='\n'.join(function(code,n) for n in ['rtl8367d_r76_table_cmd','rtl8367d_r76_write_vlan'])
  compile_run(r'''
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
typedef uint16_t u16; typedef uint32_t u32;
#define BIT(n) (1U<<(n))
#define RTL8367D_TABLE_WRITE_DATA0_REG 0x510
#define RTL8367D_TABLE_WRITE_DATA1_REG 0x511
#define RTL8367D_TABLE_ACCESS_ADDR_REG 0x501
#define RTL8367D_TABLE_ACCESS_CTRL_REG 0x500
#define RTL8367D_TABLE_ACCESS_MASK 0xf
#define RTL8367D_TABLE_ACCESS_CVLAN_WRITE 0xb
#define dev_err(...) ((void)0)
struct realtek_priv { int *map; int *dev; };
static unsigned int regs[0x600], table[4096][2];
static int calls,fail_at,busy,corrupt,commands;
static int poll(unsigned int *v) { *v=busy?BIT(13):0;return busy?-ETIMEDOUT:0; }
#define regmap_read_poll_timeout(m,a,v,c,d,t) poll(&(v))
static int regmap_read(int *m,unsigned int a,unsigned int *v) { if(++calls==fail_at)return -EIO;*v=regs[a];return 0; }
static int regmap_write(int *m,unsigned int a,unsigned int v) {
 if(++calls==fail_at)return -EIO;
 regs[a]=v;
 if(a==0x500) {
  commands++;unsigned int vid=regs[0x501];
  if((v&15)==11){table[vid][0]=regs[0x510];table[vid][1]=regs[0x511];}
  if((v&15)==3){regs[0x520]=table[vid][0]^(corrupt==1);regs[0x521]=table[vid][1]|(corrupt==2?8:0);}
 }
 return 0;
}
''' +funcs+r'''
int main(void) {
 struct realtek_priv p={0};
 assert(rtl8367d_r76_write_vlan(&p,1,139)==0);
 assert(rtl8367d_r76_write_vlan(&p,2,132)==0);
 assert(table[1][0]==0x8b8b && table[2][0]==0x8484 && commands==4);
 int total=calls/2;
 for(int i=1;i<=total;i++) { calls=0;fail_at=i;assert(rtl8367d_r76_write_vlan(&p,1,139)==-EIO); }
 fail_at=0;corrupt=1;assert(rtl8367d_r76_write_vlan(&p,1,139)==-EIO);
 corrupt=2;assert(rtl8367d_r76_write_vlan(&p,1,139)==-EIO);
 corrupt=0;busy=1;calls=0;assert(rtl8367d_r76_write_vlan(&p,1,139)==-ETIMEDOUT);assert(calls==0);
 busy=0;assert(rtl8367d_r76_write_vlan(&p,4096,1)==-EINVAL);
 assert(rtl8367d_r76_write_vlan(&p,1,256)==-EINVAL);
 return 0;
}
''')
