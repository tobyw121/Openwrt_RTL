"""Execute the RD05/R76 parser and ensure existing MW5 functions are untouched."""
import subprocess,unittest,re
from pathlib import Path
from test_wr1500_wifi6 import compile_run,function
ROOT=Path(__file__).resolve().parents[2]
FILE='target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
class RoutedGuard(unittest.TestCase):
 def test_parser_bounds_encapsulation_and_transport(self):
  code=function((ROOT/FILE).read_text(),'rtl8197f_rtk_routed_header')
  compile_run(r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint8_t u8;
#define ETH_P_REALTEK 0x8899
#define ETH_P_IP 0x0800
#define ETH_P_8021Q 0x8100
#define ETH_P_8021AD 0x88a8
#define ETH_P_PPP_DISC 0x8863
#define ETH_P_PPP_SES 0x8864
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
static unsigned int get_unaligned_be16(const u8 *p){return (p[0]<<8)|p[1];}
'''+code+r'''
int main(void){
 u8 h[160]={0};unsigned int kind;
 h[12]=0x88;h[13]=0x99;h[14]=4;h[19]=2;h[20]=8;
 h[22]=0x45;h[25]=28;h[31]=17;h[47]=8;
 assert(rtl8197f_rtk_routed_header(h,50,50,4,&kind)==0);
 for(unsigned int n=0;n<50;n++)assert(rtl8197f_rtk_routed_header(h,n,n,4,&kind)!=0);
 h[19]=4;assert(rtl8197f_rtk_routed_header(h,50,50,4,&kind)==2);
 assert(rtl8197f_rtk_routed_header(h,50,50,5,&kind)==0);h[19]=2;
 h[47]=7;assert(rtl8197f_rtk_routed_header(h,50,50,4,&kind)==3);h[47]=9;
 assert(rtl8197f_rtk_routed_header(h,50,50,4,&kind)==3);
 h[28]=0x20;assert(rtl8197f_rtk_routed_header(h,50,50,4,&kind)==0);h[28]=0;
 h[31]=6;h[25]=40;h[54]=0x50;
 assert(rtl8197f_rtk_routed_header(h,62,62,4,&kind)==0);
 h[54]=0x40;assert(rtl8197f_rtk_routed_header(h,62,62,4,&kind)==3);
 h[54]=0xf0;assert(rtl8197f_rtk_routed_header(h,62,62,4,&kind)==3);
 h[20]=0x86;h[21]=0xdd;assert(rtl8197f_rtk_routed_header(h,22,22,4,&kind)==0);
 h[20]=0x88;h[21]=0x64;assert(rtl8197f_rtk_routed_header(h,22,22,4,&kind)==0 && kind==2);
 h[20]=0x88;h[21]=0xa8;h[24]=0x81;h[25]=0;h[28]=0x86;h[29]=0xdd;
 assert(rtl8197f_rtk_routed_header(h,30,30,4,&kind)==0 && kind==1);
 for(unsigned int n=22;n<30;n++)assert(rtl8197f_rtk_routed_header(h,n,n,4,&kind)==3);
 /* Every short-header access is checked, including arbitrary tag/protocol bytes. */
 for(unsigned int seed=0;seed<256;seed++)for(unsigned int n=0;n<160;n++){
  memset(h,seed,sizeof(h));rtl8197f_rtk_routed_header(h,n,n,4,&kind);
 }
 return 0;
}
''')
 def test_board_scope_excludes_mw5_even_for_matching_dt(self):
  code=function((ROOT/FILE).read_text(),'rtl8197f_rtk_routed_guard')
  compile_run(r"""
#include <stdbool.h>
#include <assert.h>
struct rtl8197f_rtknet_priv { bool is_mw5; };
static bool rd05,r76;
static bool of_machine_is_compatible(const char *p){return rd05;}
static bool rtl8197f_rtk_is_r76(void){return r76;}
"""+code+r"""
int main(void){struct rtl8197f_rtknet_priv p={0};
 assert(!rtl8197f_rtk_routed_guard(&p));rd05=true;assert(rtl8197f_rtk_routed_guard(&p));
 rd05=false;r76=true;assert(rtl8197f_rtk_routed_guard(&p));
 p.is_mw5=true;assert(!rtl8197f_rtk_routed_guard(&p));rd05=true;assert(!rtl8197f_rtk_routed_guard(&p));
 return 0;}
""")
 def test_mw5_helpers_identical_to_prechange(self):
  old=subprocess.check_output(['git','show','0df222f142:'+FILE],cwd=ROOT,text=True)
  new=(ROOT/FILE).read_text()
  names=set(re.findall(r'\b(rtl8197f_\w*mw5\w*)\s*\(',old))
  checked=0
  for name in names:
   try:a=function(old,name);b=function(new,name)
   except ValueError:continue
   # Only compare definitions, not a call expression found first.
   if a.lstrip().startswith(('static ','bool ','int ','void ')):
    self.assertEqual(a,b,name);checked+=1
  self.assertGreater(checked,5)
