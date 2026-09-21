#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[1]
checks = []

def must(path, text, desc):
    data = (root/path).read_text(errors='replace')
    if text not in data:
        raise SystemExit(f'FAIL: {desc}: missing {text!r} in {path}')
    checks.append(desc)

def must_not(path, text, desc):
    data = (root/path).read_text(errors='replace')
    if text in data:
        raise SystemExit(f'FAIL: {desc}: forbidden {text!r} in {path}')
    checks.append(desc)

drv = Path('target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c')
tag = Path('target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c')
fw = Path('target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall')
pm = Path('target/linux/realtek/base-files/usr/sbin/mw5-portmode')
hw = Path('target/linux/realtek/base-files/usr/sbin/mw5-hwaccel')
nd = Path('target/linux/realtek/base-files/usr/sbin/mw5-netdiag')

must(drv, '#define DRV_VERSION', 'driver version declaration')
for sym in ('mw5_rx_page_pool', 'mw5_hw_csum', 'mw5_hw_sg', 'mw5_hw_tso',
            'mw5_hwlookup_enable', 'mw5_hw_napt_enable', 'mw5_extport_wlan_enable'):
    must(drv, f'static bool {sym};', f'{sym} default off')
must(drv, 'static bool mw5_hw_qos;', 'HW QoS stable default off')
must(drv, '#define RTL_RTK_MW5_GRO_FLUSH_NS\t0UL', 'GRO flush timer default off')
must(drv, '#define RTL_RTK_MW5_NAPI_DEFER_HARD_IRQS\t0', 'NAPI defer default off')
must(drv, 'priv->rx_page_pool_enabled = READ_ONCE(mw5_rx_page_pool);', 'page_pool reopen policy')
must(tag, 'skb->offload_fwd_mark = 0;', 'Linux-owned bridge forwarding')
must_not(tag, 'dsa_default_offload_fwd_mark(skb);', 'no false hardware bridge mark')
must(fw, "flow_offloading='0'", 'software flowtable boot default off')
must(fw, "flow_offloading_hw='0'", 'hardware flowtable boot default off')
must_not(pm, "flow_offloading='1'", 'portmode cannot re-enable software flowtable')
must_not(pm, "flow_offloading_hw='1'", 'portmode cannot re-enable hardware flowtable')
must(hw, 'recover_datapath()', 'no-reboot datapath recovery')
must(hw, 'performance-test)', 'isolated NIC performance test profile')
must(nd, 'wifi_station_mac()', 'wired peer excludes WLAN stations')
must(nd, 'WLAN bridge membership', 'netdiag restore repairs WLAN bridge')
print(f'MW5 stable baseline validation OK: {len(checks)} checks')
