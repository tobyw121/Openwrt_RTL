"""Exercise the integrated R76 VLAN setup against a faulting register model."""
import os,re,unittest
from pathlib import Path
from test_wr1500_wifi6 import function,compile_run
ROOT=Path(__file__).resolve().parents[2]
class R76VLAN(unittest.TestCase):
 def test_production_vlan_setup_and_command_repetition(self):
  patch=(ROOT/'target/linux/realtek/patches-6.6/343-net-dsa-realtek-r76-vlan-and-opt-in-diagnostics.patch').read_text()
  text='\n'.join(l[1:] for l in patch.splitlines() if l.startswith(('+',' ')) and not l.startswith('+++'))
  # Complete setup lives in prepared source; command/writer are fully in diff.
  source=os.environ.get('R76_DSA_SOURCE')
  if not source:self.skipTest('R76_DSA_SOURCE needed for complete production function')
  text=Path(source).read_text()
  funcs='\n'.join(function(text,n) for n in ['rtl8367d_r76_table_cmd','rtl8367d_r76_write_vlan','rtl8367d_r76_dump_vlan','rtl8367d_r76_setup_cpu_path'])
  names=sorted(set(re.findall(r'\bRTL[A-Z0-9_]+',funcs))-{'RTL8367D_VLAN_PVID_CTRL_REG','RTL8365MB_PORT_ISOLATION_REG'})
  defines='\n'.join('#define '+n+' '+str(i+1) for i,n in enumerate(names))
  # Masks are bitfields, not register addresses.
  defines+='\n#undef RTL8367D_TABLE_ACCESS_CVLAN_WRITE\n#define RTL8367D_TABLE_ACCESS_CVLAN_WRITE 11\n#undef RTL8367D_VLAN_PVID_CTRL_MASK\n#define RTL8367D_VLAN_PVID_CTRL_MASK 0xfff\n#undef RTL8367D_TABLE_ACCESS_MASK\n#define RTL8367D_TABLE_ACCESS_MASK 0xf\n#undef RTL8365MB_PORT_ISOLATION_MASK\n#define RTL8365MB_PORT_ISOLATION_MASK 0xff\n#undef RTL8365MB_CPU_PORT_MASK_MASK\n#define RTL8365MB_CPU_PORT_MASK_MASK 0xff\n'
  compile_run(r'''
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
typedef uint32_t u32; typedef uint16_t u16;
#define BIT(n) (1U<<(n))
#define READ_ONCE(x) (x)
#define dev_info(...) ((void)0)
#define dev_warn(...) ((void)0)
#define dev_err(...) ((void)0)
#define regmap_read_poll_timeout(m,a,v,c,d,t) ((v)=0,(void)(c),0)
struct realtek_priv { int *map; int *dev; };
static bool r76_wan_ingress_bypass;
static int regs[65536], calls, fail_at, commands, command_reg, address_reg, data0_reg, data1_reg, write_command;
static unsigned int table[4096][2];
static int regmap_write(int *m,int a,int v) { if(++calls==fail_at)return -EIO;regs[a]=v;if(a==command_reg) {
 commands++;
 unsigned int vid=regs[address_reg];
 if((v&15)==write_command){table[vid][0]=regs[data0_reg];table[vid][1]=regs[data1_reg];}
 if((v&15)==3){regs[0x520]=table[vid][0];regs[0x521]=table[vid][1];}
}return 0; }
static int regmap_read(int *m,int a,unsigned int *v) { if(++calls==fail_at)return -EIO;*v=regs[a];return 0; }
static int regmap_update_bits(int *m,int a,int mask,int v) { return regmap_write(m,a,(regs[a]&~mask)|(v&mask)); }
static bool rtl8367d_r76_board(struct realtek_priv *p) {return true;}
#define RTL8367D_VLAN_PVID_CTRL_REG(p) (0x600+(p))
#define RTL8365MB_PORT_ISOLATION_REG(p) (0x700+(p))
static int rtl8365mb_port_set_isolation(struct realtek_priv *p,int port,int mask) {return regmap_write(p->map,0x700+port,mask);}
''' + defines + funcs + r'''
int main(void) {
 struct realtek_priv p={0};
 command_reg=RTL8367D_TABLE_ACCESS_CTRL_REG;
 address_reg=RTL8367D_TABLE_ACCESS_ADDR_REG;
 data0_reg=RTL8367D_TABLE_WRITE_DATA0_REG;data1_reg=RTL8367D_TABLE_WRITE_DATA1_REG;
 write_command=RTL8367D_TABLE_ACCESS_CVLAN_WRITE;
 assert(rtl8367d_r76_write_vlan(&p,1,139)==0);
 assert(rtl8367d_r76_write_vlan(&p,2,132)==0);
 assert(commands==4);
 regs[RTL8365MB_CPU_PORT_MASK_REG]=128;
 assert(rtl8367d_r76_setup_cpu_path(&p)==0);
 assert(regs[RTL8367D_VLAN_INGRESS_REG]==15);
 assert(regs[RTL8367D_VLAN_PVID_CTRL_REG(2)]==2);
 assert(regs[RTL8367D_VLAN_PVID_CTRL_REG(3)]==1);
 assert(regs[0x702]==128 && regs[0x707]==15);
 for(int i=1;i<10;i++){calls=0;fail_at=i;assert(rtl8367d_r76_setup_cpu_path(&p)==-EIO);}
 fail_at=0;calls=0;r76_wan_ingress_bypass=true;
 assert(rtl8367d_r76_setup_cpu_path(&p)==0);
 assert(regs[RTL8367D_VLAN_INGRESS_REG]==11);
 return 0;
}
''')
