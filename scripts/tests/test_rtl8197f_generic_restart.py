"""Execute production reset registration for known and future board names."""
from pathlib import Path
import unittest
from test_wr1500_wifi6 import function, compile_run
ROOT = Path(__file__).resolve().parents[2]
class GenericRestart(unittest.TestCase):
    def test_soc_registration_independent_of_board_gpio(self):
        source = (ROOT/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rd05_oem_switch_preinit.c').read_text()
        init = function(source, 'rtl8197f_oem_switch_preinit').replace('__init ', '')
        compile_run(r'''
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#define __iomem
#define NULL ((void *)0)
#define RTL8197F_GPIO_BASE 0x18003500
#define RTL8197F_GPIO_SIZE 0x100
#define pr_warn(...) ((void)0)
#define pr_info(...) ((void)0)
struct device_node { int x; } node;
static const char *board;
static int has_soc=1, maps=0, registered=0, gpio_calls=0, puts=0;
static void *rtl8197f_sys_base;
static int rtl8197f_restart_nb;
static void (*pm_power_off)(void);
static void rtl8197f_poweroff(void) {}
static int of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
static struct device_node *of_find_compatible_node(void *a, void *b, const char *s)
{ assert(!strcmp(s,"realtek,rtl8197f-syscon")); return has_soc ? &node : NULL; }
static void *of_iomap(struct device_node *p, int index) { maps++; return p; }
static void of_node_put(struct device_node *p) { puts++; }
static int register_restart_handler(int *p) { registered++; return 0; }
static void rtl8197f_stop_watchdog(void) {}
static void *ioremap(unsigned long a, unsigned long n) { gpio_calls++; return &node; }
static void iounmap(void *p) {}
static void rtl8197f_rd05_switch_gpio_init(void *p) {}
static void rtl8197f_mw5_switch_gpio_init(void *p) {}
''' + init + r'''
int main(void) {
 const char *boards[]={"cudy,ax1500-r76","future,board","xiaomi,r4-rd05","tenda,nova-mw5"};
 for(int i=0;i<4;i++) {
  board=boards[i]; registered=maps=gpio_calls=puts=0;
  assert(rtl8197f_oem_switch_preinit()==0);
  assert(registered==1 && maps==1 && puts==1);
  assert(gpio_calls==(i>=2));
 }
 has_soc=0; registered=maps=gpio_calls=0;
 assert(rtl8197f_oem_switch_preinit()==0);
 assert(!registered && !maps && !gpio_calls);
 return 0;
}
''')
