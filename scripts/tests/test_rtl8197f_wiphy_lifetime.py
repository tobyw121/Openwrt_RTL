"""Execute registration failure paths and wiphy removal with mocked kernel APIs."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_mw5_stability import WLAN, c_function


class WiphyLifetimeTests(unittest.TestCase):
    def run_c(self, code, sanitize=False):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'test.c'
            binary = Path(directory) / 'test'
            source.write_text(code)
            flags = ['-fsanitize=address', '-fno-omit-frame-pointer'] if sanitize else []
            result = subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                                     '-Wno-unused-parameter',
                                     *flags, str(source), '-o', str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_out_of_order_wiphy_free_has_no_use_after_free(self):
        code = r'''
#include <assert.h>
#include <stdlib.h>
#define NDEBUG2(...) ((void)0)
struct wiphy;
struct rtknl {struct wiphy *wiphy;};
struct wiphy {struct rtknl private;};
static struct rtknl *rtk_phy[2];
static int rtk_phy_idx, frees;
static void wiphy_free(struct wiphy *p){frees++;free(p);}
'''
        code += c_function(WLAN / '8192cd_cfg80211.c', 'void rtk_free_wiphy(')
        code += r'''
int main(void){
 struct wiphy *a=calloc(1,sizeof(*a)), *b=calloc(1,sizeof(*b));
 assert(a && b);a->private.wiphy=a;b->private.wiphy=b;
 rtk_phy[0]=&a->private;rtk_phy[1]=&b->private;rtk_phy_idx=2;
 rtk_free_wiphy(&a->private); /* Not the last radio probed. */
 assert(rtk_phy_idx==1 && rtk_phy[0]==&b->private && rtk_phy[1]==NULL);
 rtk_free_wiphy(&b->private);
 assert(rtk_phy_idx==0 && rtk_phy[0]==NULL && frees==2);
 rtk_free_wiphy(NULL);assert(rtk_phy_idx==0);
 return 0;
}
'''
        self.run_c(code, sanitize=True)

    def test_netdev_registration_failure_is_not_published_as_success(self):
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
typedef unsigned char u8;
enum nl80211_iftype {NL80211_IFTYPE_STATION};
#define NLENTER ((void)0)
#define NLEXIT ((void)0)
#define NDEBUG(...) ((void)0)
#define FALSE false
#define WIPHY_FLAG_4ADDR_AP 1
#define WIPHY_FLAG_4ADDR_STATION 2
struct wiphy {int flags;};
struct wireless_dev {struct wiphy *wiphy;void *netdev;int iftype;};
struct net_device {char name[32];struct wireless_dev *ieee80211_ptr;};
struct rtl8192cd_priv {struct net_device *dev;struct wireless_dev wdev;bool cfg80211_interface_add;};
struct rtknl {struct wiphy *wiphy;struct net_device *ndev_add;};
static int result, published, phase_rc;
static void realtek_change_iftype(struct rtl8192cd_priv *p,int t){(void)p;(void)t;}
#define dev_net_set(d,n) ((void)(d))
#define SET_NETDEV_DEV(d,p) ((void)(d))
#define SET_NETDEV_DEVTYPE(d,t) ((void)(d))
static void free_netdev(struct net_device *p){(void)p;}
static int register_netdev(struct net_device *p){(void)p;return result;}
static void rtk_add_priv(struct rtl8192cd_priv *p,struct rtknl *r){(void)p;(void)r;published++;}
static void rtl8197f_wlan_phase_set(int n,const char *s,int rc){(void)n;(void)s;phase_rc=rc;}
'''
        code += c_function(WLAN / '8192cd_cfg80211.c', 'int realtek_interface_add(')
        code += r'''
int main(void){
 struct wiphy w={0};struct rtknl r={&w,NULL};struct net_device d={0};
 struct rtl8192cd_priv p={.dev=&d};
 result=-17;
 assert(realtek_interface_add(&p,&r,"wlan0",NL80211_IFTYPE_STATION,0,0)==-17);
 assert(!r.ndev_add && !published && phase_rc==-17);
 result=0;
 assert(realtek_interface_add(&p,&r,"wlan0",NL80211_IFTYPE_STATION,0,0)==0);
 assert(r.ndev_add==&d && published==1 && phase_rc==0);
 return 0;
}
'''
        self.run_c(code)

    def test_wiphy_and_regulatory_errors_unwind_in_order(self):
        function = c_function(WLAN / '8192cd_cfg80211.c', 'int realtek_cfg80211_init(')
        tail = function[function.rindex('\twiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;'):]
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#define EINVAL 22
#define REGULATORY_WIPHY_SELF_MANAGED 1
#define NLEXIT ((void)0)
struct ieee80211_regdomain {int unused;};
struct wiphy {int regulatory_flags;};
struct rtknl {struct wiphy *wiphy;bool wiphy_registered;};
struct mib {struct {int dot11RegDomain;} dot11StationConfigEntry;};
struct priv {struct mib *pmib;};
static struct mib m;
static struct priv p={&m};
static struct ieee80211_regdomain domain;
static int null_domain, reg_result, domain_result, reg_calls, domain_calls, unreg_calls, locks;
static const struct ieee80211_regdomain *_rtl_regdomain_select(int n){(void)n;return null_domain?NULL:&domain;}
static int wiphy_register(struct wiphy *w){(void)w;reg_calls++;return reg_result;}
static bool rtnl_is_locked(void){return false;}
static void rtnl_lock(void){locks++;}
static void rtnl_unlock(void){locks--;}
static int regulatory_set_wiphy_regd_sync(struct wiphy *w,struct ieee80211_regdomain *d){
 (void)w;assert(d && reg_calls && !reg_result && locks==1);domain_calls++;return domain_result;
}
static void rtl8197f_wlan_phase_set(int n,const char *s,int rc){(void)n;(void)s;(void)rc;}
static void rtk_unregister_wiphy(struct rtknl *r){assert(r->wiphy_registered);unreg_calls++;r->wiphy_registered=false;}
static int run_registration(struct rtknl *rtk){
 struct wiphy *wiphy=rtk->wiphy;
 struct priv *priv=&p;
 const struct ieee80211_regdomain *realtek_regdom;
 int ret;
'''
        code += tail + r'''
int main(void){
 struct wiphy w={0};struct rtknl r={&w,false};
 null_domain=1;assert(run_registration(&r)==-22 && reg_calls==0);
 null_domain=0;reg_result=-12;assert(run_registration(&r)==-12);
 assert(!r.wiphy_registered && domain_calls==0 && unreg_calls==0);
 reg_result=0;domain_result=-22;assert(run_registration(&r)==-22);
 assert(!r.wiphy_registered && domain_calls==1 && unreg_calls==1 && locks==0);
 domain_result=0;assert(run_registration(&r)==0);
 assert(r.wiphy_registered && domain_calls==2 && unreg_calls==1 && locks==0);
 return 0;
}
'''
        self.run_c(code)


if __name__ == '__main__':
    unittest.main()
