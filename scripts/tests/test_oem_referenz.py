"""Verify original GPL references and execute the production reset fallback."""
import hashlib,json,unittest
from pathlib import Path
from test_wr1500_wifi6 import compile_run,function
ROOT=Path(__file__).resolve().parents[2]
class OEMReference(unittest.TestCase):
 def test_manifest_integrity(self):
  ref=ROOT/'OEM_referenz';data=json.loads((ref/'manifest.json').read_text())
  self.assertEqual({r['source'] for r in data['files']},{'google_ax1500','tplink_ax1500v3'})
  for r in data['files']:
   p=ref/r['path'];self.assertTrue(p.resolve().is_relative_to(ref.resolve()))
   self.assertEqual(hashlib.sha256(p.read_bytes()).hexdigest(),r['sha256'],r['path'])
 def test_watchdog_fallback_after_software_reset(self):
  s=(ROOT/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rd05_oem_switch_preinit.c').read_text()
  defines='\n'.join(l for l in s.splitlines() if l.startswith('#define '))
  code=r'''
#include <stdint.h>
#include <assert.h>
typedef uint32_t u32;
#define BIT(n) (1U<<(n))
#define pr_emerg(...) ((void)0)
static unsigned char memory[65536];
static void *rtl8197f_sys_base=memory;
static int writes,delays;static uintptr_t last_addr;static unsigned int last_value;
static unsigned int readl(void *p){return 0;}
static void writel(unsigned int v,void *p){writes++;last_addr=(uintptr_t)p;last_value=v;}
static void udelay(int n){}
static void mdelay(int n){assert(n==2500);delays++;}
'''+defines+'\n'+function(s,'rtl8197f_assert_reset')+r'''
int main(void){
 rtl8197f_assert_reset();assert(delays==2);
 assert(last_value==0 && last_addr==(uintptr_t)(memory+RTL8197F_WDTCNR));
 rtl8197f_sys_base=0;int count=writes;rtl8197f_assert_reset();assert(writes==count);
 return 0;
}
'''
  compile_run(code)
