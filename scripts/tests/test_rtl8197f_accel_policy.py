#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Strict policy tests: no forwarding shortcut and no host firewall mutations."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, BASE, c_function

DRIVER = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
TOOL = BASE / 'usr/sbin/rtl8197f-accel'


class StrictAccelerationTests(unittest.TestCase):
    def test_hardware_nat_port_edits_match_linux_nf_flow_table_masks(self):
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
typedef uint32_t u32;
typedef uint32_t __be32;
typedef uint16_t __be16;
#define __force
#define be32_to_cpu ntohl
#define cpu_to_be16 htons
struct flow_action_entry { struct { u32 offset, val, mask; } mangle; };
''' + c_function(DRIVER, 'static int rtl8197f_rtk_napt_mangle_port(') + r'''
int main(void) {
    __be16 src=htons(1234), dst=htons(4321);
    struct flow_action_entry a={.mangle={0,htonl(54321U<<16),htonl(0x0000ffff)}};
    assert(!rtl8197f_rtk_napt_mangle_port(&a,&src,&dst));
    assert(ntohs(src)==54321 && ntohs(dst)==4321);
    a.mangle.val=htonl(65432); a.mangle.mask=htonl(0xffff0000);
    assert(!rtl8197f_rtk_napt_mangle_port(&a,&src,&dst));
    assert(ntohs(src)==54321 && ntohs(dst)==65432);
    u32 bad[]={0,0xffffffff,0xffff00ff,0x00ffffff};
    for (unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++) {
        a.mangle.val=0; a.mangle.mask=htonl(bad[i]);
        assert(rtl8197f_rtk_napt_mangle_port(&a,&src,&dst)==-EOPNOTSUPP);
        assert(ntohs(src)==54321 && ntohs(dst)==65432);
    }
    a.mangle.mask=htonl(0x0000ffff); a.mangle.val=htonl(0x12340001);
    assert(rtl8197f_rtk_napt_mangle_port(&a,&src,&dst)==-EOPNOTSUPP);
    a.mangle.val=htonl(0x12340000); a.mangle.offset=2;
    assert(rtl8197f_rtk_napt_mangle_port(&a,&src,&dst)==-EOPNOTSUPP);
    assert(ntohs(src)==54321 && ntohs(dst)==65432);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'test.c'; exe=Path(tmp)/'test'
            c.write_text(code)
            subprocess.run(['cc','-std=gnu11','-Werror','-fsanitize=undefined',str(c),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)

    def test_kernel_rejects_optins_and_keeps_packet_on_linux_path(self):
        names = ['static bool rtl8197f_rtk_forward_offload_allowed(',
                 'static int rtl8197f_rtk_forward_param_set(',
                 'static void rtl8197f_rtk_napt_set_l34(',
                 'static int rtl8197f_rtk_setup_tc(',
                 'int rtl8197f_mw5_accel_rx(struct sk_buff *skb)\n{']
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
typedef uint32_t u32;
#define EOPNOTSUPP 95
#define READ_ONCE(x) (x)
#define RTL_RTK_SWCORE_MSCR 0
#define RTL_RTK_MSCR_EN_L2 1
#define RTL_RTK_MSCR_EN_L3 2
#define RTL_RTK_MSCR_EN_L4 4
struct kernel_param { bool *arg; };
static int kstrtobool(const char *s, bool *v) {
    if (!strcmp(s,"1") || !strcmp(s,"true")) *v=true;
    else if (!strcmp(s,"0") || !strcmp(s,"false")) *v=false;
    else return -EINVAL;
    return 0;
}
static int param_set_bool(const char *s, const struct kernel_param *p) { return kstrtobool(s,p->arg); }
struct rtl8197f_rtknet_priv { void *swcore; int tc_unsupported; };
struct net_device { struct rtl8197f_rtknet_priv *priv; }; struct sk_buff {};
enum tc_setup_type { TC_SETUP_BLOCK, TC_SETUP_FT, OTHER };
static struct rtl8197f_rtknet_priv *rtl8197f_mw5_runtime;
static int forward_calls, block_calls;
static u32 readl(void *p) { return *(u32 *)p; }
static void writel(u32 v, void *p) { *(u32 *)p=v; }
static struct rtl8197f_rtknet_priv *netdev_priv(struct net_device *d) { return d->priv; }
static int rtl8197f_rtk_setup_tc_block(struct net_device *d, void *p, bool flow) {
    assert(!flow); block_calls++; return -EOPNOTSUPP;
}
static int rtl8197f_mw5_accel_sw_forward(struct rtl8197f_rtknet_priv *p, struct sk_buff *s) {
    forward_calls++; return 1;
}
''' + '\n'.join(c_function(DRIVER, n) for n in names) + r'''
int main(void) {
    bool enabled=false; struct kernel_param kp={&enabled};
    assert(rtl8197f_rtk_forward_param_set("1",&kp)==-EOPNOTSUPP && !enabled);
    assert(rtl8197f_rtk_forward_param_set("true",&kp)==-EOPNOTSUPP && !enabled);
    enabled=true;
    assert(rtl8197f_rtk_forward_param_set("0",&kp)==0 && !enabled);
    assert(rtl8197f_rtk_forward_param_set("garbage",&kp)==-EINVAL && !enabled);
    u32 mscr=0xffffffff;
    struct rtl8197f_rtknet_priv priv={.swcore=&mscr};
    rtl8197f_rtk_napt_set_l34(&priv,true);
    assert(mscr==0xfffffff9); /* Only L3/L4 cleared, L2 and other bits preserved. */
    rtl8197f_mw5_runtime=&priv;
    assert(rtl8197f_mw5_accel_rx((void *)1)==0 && forward_calls==0);
    assert(rtl8197f_mw5_accel_rx(0)==0 && forward_calls==0);
    struct net_device dev={&priv};
    assert(rtl8197f_rtk_setup_tc(&dev,TC_SETUP_FT,0)==-EOPNOTSUPP);
    assert(block_calls==0 && priv.tc_unsupported==1);
    assert(rtl8197f_rtk_setup_tc(&dev,TC_SETUP_BLOCK,0)==-EOPNOTSUPP);
    assert(block_calls==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'test.c'; exe=Path(tmp)/'test'
            c.write_text(code)
            subprocess.run(['cc','-std=gnu11','-Werror','-fsanitize=undefined',str(c),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)

    def run_policy(self, command, *, board='tenda,nova-mw5', nft='absent', section='defaults',
                   driver=True, software='0', modules=True, wan=True, check=True, reload=True):
        source = TOOL.read_text().split('case "${1:-status}" in',1)[0]
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp); proc=path/'proc'; log=path/'log'
            proc.write_text('firewall_policy mode=strict routed_shortcuts=0 tc_flowtable_hw=0 dma_path=linux\n' if driver else 'old driver\n')
            source += r'''
PROC="$TEST_PROC"
cat() { echo "$TEST_BOARD"; }
uci() {
    [ "$1" != -q ] || shift
    case "$1:$2" in
        get:firewall.@defaults\[0\]) echo "$TEST_SECTION" ;;
        get:firewall.@defaults\[0\].flow_offloading) echo "$TEST_SOFTWARE" ;;
        get:firewall.@defaults\[0\].flow_offloading_hw) echo 0 ;;
        *) echo "$*" >> "$TEST_LOG" ;;
    esac
}
firewall_reload() { echo reload >> "$TEST_LOG"; [ "$TEST_RELOAD" = 1 ]; }
software_modules_available() { [ "$TEST_MODULES" = 1 ]; }
ifstatus() { printf '{"up": %s}\n' "$TEST_WAN"; }
fw4() { echo "fw4 $*" >> "$TEST_LOG"; [ "$TEST_CHECK" = 1 ]; }
nft() {
    case "$TEST_NFT" in
        absent) echo 'table inet fw4 { chain forward { type filter hook forward priority filter; } }' ;;
        unused) echo 'table inet fw4 { chain forward { type filter hook forward priority filter; } flowtable ft { hook ingress priority filter; } }' ;;
        nohook) echo 'table inet fw4 { }' ;;
        foreign)
            echo 'table inet fw4 { chain forward { type filter hook forward priority filter; } }'
            [ "$*" != 'list ruleset' ] || echo 'table inet foreign { flow add @foreign_ft }' ;;
        present) echo 'table inet fw4 { chain forward { type filter hook forward priority filter; flow add @ft } }' ;;
        legacy) echo 'flow offload @ft' ;;
        *) return 1 ;;
    esac
}
'''
            result=subprocess.run(['sh','-c',source+'\n'+command],text=True,capture_output=True,
                env=dict(os.environ,TEST_PROC=str(proc),TEST_LOG=str(log),TEST_BOARD=board,
                         TEST_NFT=nft,TEST_SECTION=section,TEST_SOFTWARE=software,
                         TEST_MODULES='1' if modules else '0',TEST_WAN='true' if wan else 'false',
                         TEST_CHECK='1' if check else '0',TEST_RELOAD='1' if reload else '0'))
            return result, log.read_text() if log.exists() else ''

    def test_configuration_disables_offload_without_replacing_firewall(self):
        for board in ('tenda,nova-mw5','xiaomi,r4-rd05','tenda,ac23','iball,wrd12gn','cudy,wr1500'):
            result,log=self.run_policy('configure && firewall_reload',board=board)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(log.splitlines(),['set firewall.@defaults[0].flow_offloading=0',
                'set firewall.@defaults[0].flow_offloading_hw=0','commit firewall','reload'])
        for args in ({'board':'unrelated,switch'}, {'section':'missing'}):
            result,log=self.run_policy('configure',**args)
            self.assertNotEqual(result.returncode,0)
            self.assertEqual(log,'')

    def test_verification_rejects_active_rules_unknown_nft_and_old_driver(self):
        for nft,driver,success in [('absent',True,True),('unused',True,True),
            ('present',True,False),('legacy',True,False),('unknown',True,False),
            ('nohook',True,False),('absent',False,False)]:
            with self.subTest(nft=nft,driver=driver):
                result,log=self.run_policy('verify',nft=nft,driver=driver)
                self.assertEqual(result.returncode==0,success,result.stdout+result.stderr)
                self.assertEqual(log,'')

    def test_software_offload_is_explicit_and_firewall4_owned(self):
        result,log=self.run_policy('apply_mode 1',board='tenda,ac23')
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(log.splitlines(),['set firewall.@defaults[0].flow_offloading=1',
            'set firewall.@defaults[0].flow_offloading_hw=0','commit firewall','fw4 check','reload'])
        result,log=self.run_policy('verify',board='tenda,ac23',software='1',nft='present')
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertIn('software_flowtable_policy=PASS',result.stdout)
        self.assertEqual(log,'')
        result,_=self.run_policy('verify',board='tenda,ac23',software='1',nft='foreign')
        self.assertNotEqual(result.returncode,0)

    def test_unavailable_datapaths_do_not_mutate_firewall(self):
        cases=[{'board':'tenda,nova-mw5'},{'board':'cudy,wr1500'},
               {'board':'xiaomi,r4-rd05','wan':False},{'board':'tenda,ac23','modules':False},
               {'board':'tenda,ac23','nft':'unknown'},{'board':'tenda,ac23','driver':False}]
        for args in cases:
            with self.subTest(args=args):
                result,log=self.run_policy('apply_mode 1',**args)
                self.assertNotEqual(result.returncode,0)
                self.assertEqual(log,'')

    def test_failed_validation_or_reload_restores_previous_offload_options(self):
        for args in ({'check':False},{'reload':False}):
            with self.subTest(args=args):
                result,log=self.run_policy('apply_mode 1',board='tenda,ac23',**args)
                self.assertNotEqual(result.returncode,0)
                self.assertIn('set firewall.@defaults[0].flow_offloading=1',log)
                self.assertIn('set firewall.@defaults[0].flow_offloading=0',log)
                if not args.get('check',True):
                    self.assertNotIn('reload',log)
                else:
                    self.assertEqual(log.splitlines().count('reload'),2)


if __name__ == '__main__':
    unittest.main()
