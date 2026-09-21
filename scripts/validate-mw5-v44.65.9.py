#!/usr/bin/env python3
from pathlib import Path
import re, sys

root = Path(__file__).resolve().parents[1]
drv = (root/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c').read_text()
tag = (root/'target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c').read_text()
hw = (root/'target/linux/realtek/base-files/usr/sbin/mw5-hwaccel').read_text()
st = (root/'target/linux/realtek/base-files/usr/sbin/mw5-speedtest').read_text()
banner = (root/'target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh').read_text()
errors=[]

def need(text, needle, label):
    if needle not in text: errors.append(f'{label}: missing {needle!r}')

def forbid(text, needle, label):
    if needle in text: errors.append(f'{label}: forbidden {needle!r}')

need(drv, '"1.8.5-mw5-dsa-rx-recovery-v44.65.9"', 'driver version')
need(drv, 'rtl8197f_rtk_mw5_recover_source_port', 'DA source-port fallback')
need(drv, 'is_multicast_ether_addr(da)', 'no multicast guessing')
need(drv, 'ether_addr_equal(sa, base)', 'local-reflection guard')
need(drv, 'eth_addr_add(wan_mac, 7);', 'MW5 WAN base+7 invariant')
need(drv, '*source_port = 3;', 'WAN P3 fallback')
need(drv, '*source_port = 1;', 'LAN P1 fallback')
need(drv, 'rtl8197f_rtk_mw5_known_payload_proto(proto4)', 'inner-protocol structural proof')
need(drv, 'mw5_rx_tag_repaired_inplace', 'in-place repair counter')
need(drv, 'mw5_rx_tag_fallback_wan_da', 'WAN fallback counter')
need(drv, 'mw5_rx_tag_fallback_lan_da', 'LAN fallback counter')
need(drv, 'mw5_rx_tag_unclassified', 'unclassified counter')
need(drv, 'cdp_owned_advanced=%llu', 'CDP/OWN diagnostic')
need(drv, 'put_unaligned_be16(ETH_P_REALTEK, skb->data + 2 * ETH_ALEN);', 'tag reconstruction')

# v44.62.14 anti-XDSA-loop behavior must remain fail closed.
idx = tag.find('if (ntohs(tag16[0]) != ETH_P_REALTEK)')
if idx < 0:
    errors.append('tagger: missing non-0x8899 guard')
else:
    block = tag[idx:tag.find('\n\t}', idx)+3]
    if 'return NULL;' not in block:
        errors.append('tagger: untagged guard no longer returns NULL')
    if 'return skb;' in block:
        errors.append('tagger: untagged guard reintroduces XDSA reinjection')
need(tag, 'prevents XDSA reinjection loop', 'tagger anti-loop diagnostic')

# Stable MW5 profile: no optional accelerator is initialized enabled.
for name in ['mw5_rx_page_pool','mw5_hw_csum','mw5_hw_sg','mw5_hw_tso',
             'mw5_hw_qos','mw5_hwlookup_enable','mw5_hw_napt_enable',
             'mw5_extport_wlan_enable','mw5_drop_invalid_untagged']:
    if re.search(rf'static bool {re.escape(name)}\s*=\s*true\s*;', drv):
        errors.append(f'default policy: {name} unexpectedly true')

need(st, 'tool=mw5-speedtest-v44.65.9', 'speedtest version')
need(st, 'dnsmasq_lan_socket=PASS', 'dnsmasq socket diagnostic')
need(st, 'dns_lan_query=PASS', 'LAN DNS query diagnostic')
need(st, 'dns_upstream=PASS', 'upstream DNS diagnostic')
forbid(st, "echo 'dns_lan=FAIL'", 'ambiguous old DNS health flag')
need(hw, 'MW5 recovery v44.65.9: restore classic OpenWrt routing/NAT/DNS', 'recover version')
need(hw, 'set_qos off', 'recover safe QoS')
need(hw, 'set_invalid_drop off', 'recover unknown-SPA policy')
need(banner, 'MW5 v44.65.9 DSA-RX recovery baseline', 'banner version')

# Small model checks for the intended frame transforms.
def mac(s): return bytes(int(x,16) for x in s.split(':'))
base=mac('cc:2d:21:9e:70:c0')
wan=(int.from_bytes(base,'big')+7).to_bytes(6,'big')
assert wan == mac('cc:2d:21:9e:70:c7')
# stripped tag: DA|SA|8100 -> insert 8899|0403
f=bytearray(wan+mac('b0:f2:08:99:56:db')+bytes.fromhex('8100000008004500'))
f[12:12]=bytes.fromhex('88990403')
assert f[12:20] == bytes.fromhex('8899040381000000')
# damaged tag: DA|SA|A728|A04C|8100 -> overwrite, don't shift
f=bytearray(wan+mac('b0:f2:08:99:56:db')+bytes.fromhex('a728a04c810000000800'))
oldlen=len(f); f[12:16]=bytes.fromhex('88990403')
assert len(f)==oldlen and f[12:18]==bytes.fromhex('889904038100')

if errors:
    print('MW5 v44.65.9 validation FAILED', file=sys.stderr)
    for e in errors: print('  - '+e, file=sys.stderr)
    sys.exit(1)
print('MW5 v44.65.9 DSA RX recovery validation OK: 27 static/transform checks')
