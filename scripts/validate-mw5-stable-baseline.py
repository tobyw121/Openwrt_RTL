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
# MW5 moved to fixed coherent DMA slots; the retired page_pool parameter
# must not put the streaming/page_pool ownership path back into use.
mw5_open = (root/drv).read_text().split('static int rtl8197f_rtk_open(', 1)[1].split(
    'static int rtl8197f_rtk_stop(', 1)[0]
for statement in ('priv->rx_coherent_enabled = true;',
                  'priv->tx_coherent_enabled = true;',
                  'priv->rx_page_pool_enabled = false;'):
    if statement not in mw5_open:
        raise SystemExit(f'FAIL: MW5 coherent DMA reopen policy: missing {statement!r}')
    checks.append('MW5 coherent DMA reopen policy: ' + statement)
must_not(drv, 'rx_page_pool_enabled = READ_ONCE(mw5_rx_page_pool)',
         'retired MW5 page_pool ownership path')
must(tag, 'skb->offload_fwd_mark = 0;', 'Linux-owned bridge forwarding')
must_not(tag, 'dsa_default_offload_fwd_mark(skb);', 'no false hardware bridge mark')
must(fw, "flow_offloading='0'", 'software flowtable boot default off')
must(fw, "flow_offloading_hw='0'", 'hardware flowtable boot default off')
must_not(pm, "flow_offloading='1'", 'portmode cannot re-enable software flowtable')
must_not(pm, "flow_offloading_hw='1'", 'portmode cannot re-enable hardware flowtable')
must(hw, 'exec /usr/sbin/mw5-accel status', 'legacy status forwards to current controls')
must(hw, 'legacy controls are disabled', 'legacy acceleration controls remain retired')
must_not(hw, 'recover_datapath()', 'no legacy live DMA reset command')
must(nd, 'wifi_station_mac()', 'wired peer excludes WLAN stations')
must(nd, 'WLAN bridge membership', 'netdiag restore repairs WLAN bridge')
print(f'MW5 stable baseline validation OK: {len(checks)} checks')
