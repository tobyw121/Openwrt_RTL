#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Static regression guard for MW5 OEM V212 v44.66.6."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DRV = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
DTS = ROOT / 'target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts'
TAG = ROOT / 'target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c'
FW = ROOT / 'target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall'
ACC = ROOT / 'target/linux/realtek/base-files/etc/uci-defaults/97-mw5-accel'
PATCH = ROOT / 'target/linux/realtek/patches-6.6/328-net-dsa-realtek-rtl8365mb-add-mw5-rtl8363-family.patch'
PATCH329 = ROOT / 'target/linux/realtek/patches-6.6/329-net-dsa-realtek-add-rtl4-protocol9-for-mw5.patch'
CPU_PATCH = ROOT / 'target/linux/realtek/patches-6.6/330-net-dsa-realtek-mw5-fix-cpu-egress-isolation.patch'
PATCH331 = ROOT / 'target/linux/realtek/patches-6.6/331-net-dsa-realtek-mw5-v43-15-strip-leaked-cpu-tags.patch'
ROUTER_AUDIT = ROOT / 'MW5-LIVE-AUDIT/mw5-router-audit.sh'
PC_AUDIT = ROOT / 'MW5-LIVE-AUDIT/mw5-pc-audit-interactive.sh'
PCAP_ANALYZER = ROOT / 'MW5-LIVE-AUDIT/mw5-pcap-compare.py'
ACCESS = ROOT / 'target/linux/realtek/base-files/etc/init.d/mw5-access'
PORTMODE = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-portmode'
WANLAN = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-wan-lan'
NETDIAG = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-netdiag'
STATUS = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-access-status'
FEEDS = ROOT / 'scripts/feeds'

errors = []
def load(path): return path.read_text(errors='replace')
def need(path, needle, label):
    if needle not in load(path): errors.append(f'{label}: missing {needle!r} in {path.relative_to(ROOT)}')
def forbid(path, needle, label):
    if needle in load(path): errors.append(f'{label}: forbidden {needle!r} in {path.relative_to(ROOT)}')

# V212 geometry / descriptor ABI.
need(DRV, '#define DRV_VERSION\t\t\t"1.8.28-mw5-oem-v212-v44.66.6"', 'driver version')
need(DRV, '#define RTL_RTK_MW5_OEM_TX_RING0\t768', 'OEM TX ring0')
need(DRV, '#define RTL_RTK_MW5_OEM_RX_RING0\t900', 'OEM RX ring0')
need(DRV, 'priv->tx_ring_size = RTL_RTK_MW5_OEM_TX_RING0;', 'board TX ring override')
need(DRV, 'priv->rx_ring_size = RTL_RTK_MW5_OEM_RX_RING0;', 'board RX ring override')
need(DTS, 'realtek,descriptor-dwords = <6>;', '6-DWORD descriptor DTS')
need(DTS, 'realtek,tx-ring-size = <768>;', 'TX ring DTS')
need(DTS, 'realtek,rx-ring-size = <900>;', 'RX ring DTS')

# OEM-shaped fail-closed completion and coherent lifetime fence.
need(DRV, 'tx_head is the Linux/OEM', 'OEM txCurrIdx mapping')
need(DRV, 'CPUTPDCR0 is the hardware CDP', 'hardware CDP completion')
need(DRV, 'if (idx == hw_idx)', 'CDP boundary')
need(DRV, 'if (confirm & RTL_RTK_DESC_OWN)', 'OWN completion fence')
need(DRV, 'mw5_tx_cdp_invalid_defer++', 'invalid CDP fail closed')
forbid(DRV, 'reclaim_lag=8', 'estimated reclaim distance removed')
forbid(DRV, 'reclaim_lag = 8', 'estimated reclaim distance removed')
need(DRV, 'dma_pool_create("rtl8197f-mw5-tx"', 'coherent TX pool')
need(DRV, 'memcpy(priv->tx_coherent_cpu[idx], skb->data, skb->len);', 'coherent TX copy')
need(DRV, 'reuse_opts1 & RTL_RTK_DESC_OWN', 'coherent slot OWN reuse guard')
need(DRV, 'READ_ONCE(priv->tx_skb[idx])', 'coherent slot SW reuse guard')

# v44.66.6 corrected physical topology: a real RTL8367 tag stays on wire and
# the RTL8197F master always transports it through P0/RGMII.
need(TAG, 'static unsigned int rtl4_9_tx_layout = RTL4_9_TX_LAYOUT_NATIVE0400;', 'RTL8367 4-byte tag default')
need(TAG, 'RTL4_9_TX_NATIVE_MARKER |', 'native 0x0400 tag encoding')
need(TAG, 'FIELD_PREP(RTL4_9_TX_NATIVE_PORT, port_mask)', 'one-hot external target mask')
need(DRV, 'static bool rtl8197f_rtk_mw5_wire_cputag', 'wire CPU-tag recognizer')
need(DRV, '(tag & RTL_RTK_MW5_TX_NATIVE_MASK) != RTL_RTK_MW5_TX_NATIVE_MARKER', 'native tag recognition')
need(DRV, 'txm.port_mask = BIT(0);', 'physical P0 descriptor target')
need(DRV, 'txm.vid_valid = false;', 'no internal DVID on normal wire-tag path')
need(DRV, 'priv->mw5_tx_wiretag_lan++', 'LAN wire-tag diagnostic')
need(DRV, 'priv->mw5_tx_wiretag_wan++', 'WAN wire-tag diagnostic')
need(DRV, 'normal_dp=p0', 'proc physical P0 invariant')
# Keep layout3 only as a diagnostic reconstruction path, not the default.
need(TAG, 'RTL4_9_TX_LAYOUT_DESC_META', 'descriptor-meta diagnostic retained')
forbid(TAG, 'static unsigned int rtl4_9_tx_layout = RTL4_9_TX_LAYOUT_DESC_META;', 'layout3 must not be default')

# v44.66.6 build regression: patch 329 Kconfig hunk has 9 inserted lines.
need(PATCH329, '@@ -130,6 +130,15 @@ config NET_DSA_TAG_RTL4_A', 'patch329 Kconfig hunk accounting')
forbid(PATCH329, '@@ -130,6 +130,14 @@ config NET_DSA_TAG_RTL4_A', 'malformed patch329 hunk removed')

need(CPU_PATCH, '@@ -1206,6 +1208,175 @@ static int rtl8365mb_port_set_isolation', 'patch330 hunk accounting')
forbid(CPU_PATCH, '@@ -1206,6 +1208,177 @@ static int rtl8365mb_port_set_isolation', 'malformed patch330 hunk removed')

# HW lookup/NAPT remain fail-closed for normal routing.
need(FW, "flow_offloading_hw='0'", 'hardware flowtable disabled')
need(ACC, "flow_offloading_hw='0'", 'accelerator hardware flowtable disabled')
need(DRV, 'priv->hw_napt = false;', 'legacy HW NAPT disabled')
need(DRV, 'priv->tx_hwlookup = false;', 'legacy HWLOOKUP disabled')

# Never copy internal RTL8197F V212 flow-control values into unknown external regs.
need(PATCH, 'No unknown flow-control register is written from V212.', 'flow-control domain guard')
for reg in range(0x121f, 0x1227):
    forbid(PATCH, f'regmap_write(priv->map, 0x{reg:04x}', f'no guessed external flow-control write 0x{reg:04x}')

# External CPU parser: CPU port only; mode/position are observed, not guessed.
for needle, label in (
    ('#define RTL8365MB_MW5_CPU_EN\t\t\tBIT(0)', 'CPU tag enable bit'),
    ('#define RTL8365MB_MW5_CPU_INSERT_MODE\t\tGENMASK(2, 1)', 'CPU insert-mode bits'),
    ('#define RTL8365MB_MW5_CPU_TRAP_PORT\t\tGENMASK(5, 3)', 'CPU trap-port bits'),
    ('#define RTL8365MB_MW5_CPU_TAG_POSITION\t\tBIT(6)', 'CPU tag position bit'),
    ('#define RTL8365MB_MW5_CPU_RXBYTECOUNT\t\tBIT(7)', 'CPU RX-bytecount bit'),
    ('#define RTL8365MB_MW5_CPU_TAG_FORMAT\t\tBIT(9)', 'CPU 4-byte format bit'),
    ('RTL8365MB_CPU_PORT_MASK_REG', 'CPU parser mask register'),
    ('no guessed tag-mode/position values are written here', 'no guessed CPU tag mode writes'),
): need(CPU_PATCH, needle, label)
need(CPU_PATCH, 'four-byte CPU tag after the frame arrives over RTL8197F P0/RGMII', 'external selection follows P0 wire tag')
need(PATCH331, 'mw5_diag=v44.66.6', 'patch331 diagnostic revision')
need(PATCH331, 'cpu_tag_policy=v44.66.6', 'patch331 CPU-tag policy revision')

# MIPS -Werror=format regression stays fixed.
for cast in (
    '(unsigned int)((d0 & RTL_RTK_TX_TYPE_MASK) >> RTL_RTK_TX_TYPE_SHIFT)',
    '(unsigned int)((d0 & RTL_RTK_TX_PH_LEN_MASK) >> RTL_RTK_TX_PH_LEN_SHIFT)',
    '(unsigned int)((d2 & RTL_RTK_TX_M_LEN_MASK) >> RTL_RTK_TX_M_LEN_SHIFT)',
    '(unsigned int)(d3 & RTL_RTK_TX_DVID_MASK)',
): need(DRV, cast, 'MIPS-safe descriptor diagnostic cast')
need(FEEDS, "git -C '$safepath' config core.fileMode false", 'feed fileMode normalization')

# Every normal userspace entry point must keep layout1; legacy layouts are opt-in.
need(ACCESS, 'echo 1 >/sys/module/tag_rtl4_9/parameters/tx_layout', 'boot enforces layout1')
forbid(ACCESS, 'echo 3 >/sys/module/tag_rtl4_9/parameters/tx_layout', 'boot must not force layout3')
need(PORTMODE, 'force_rtl8367_cputag_tx()', 'portmode layout1 helper')
need(PORTMODE, 'echo 1 >/sys/module/tag_rtl4_9/parameters/tx_layout', 'portmode enforces layout1')
need(WANLAN, '[ "$tx_layout" = 1 ]', 'WAN/LAN test requires layout1')
need(STATUS, 'tx_layout_expected=1', 'status expects layout1')
need(NETDIAG, 'selected_layout=1', 'netdiag restores layout1')
need(NETDIAG, 'MW5_DIAG_LEGACY_TAG_AB', 'A/B diagnostics explicit opt-in')

# Live Audit v7.2 persists evidence and refuses a normal stress test on a
# diagnostic layout.
for key in ('mw5_oem_geometry', 'mw5_rx_oem_state', 'mw5_tx_oem_state', 'mw5_oem_vlan', 'mw5_oem_flowctrl'):
    need(DRV, key, f'/proc diagnostic {key}')
need(ROUTER_AUDIT, 'collector v7.3/v44.66.6', 'router audit revision')
need(ROUTER_AUDIT, 'expected_tx_layout=1', 'router audit expects layout1')
need(ROUTER_AUDIT, 'live-stream) live_stream "$2"', 'router persistent live stream')
need(PC_AUDIT, 'pc_audit=v7.3-v44.66.6', 'PC audit revision')
need(PC_AUDIT, 'expected_tx_layout=1', 'PC audit expects layout1')
need(PC_AUDIT, 'ControlMaster=auto', 'SSH multiplexing')
need(PC_AUDIT, 'SNAPLEN=2048', 'full Ethernet payload PC capture')
need(PC_AUDIT, 'mw5-pcap-compare.py', 'automatic physical PCAP comparison')
need(PCAP_ANALYZER, 'exact_cross_port_duplicates', 'cross-port duplicate detector')
need(PCAP_ANALYZER, 'payload_as_header_candidates', 'payload-as-header detector')

if errors:
    print('MW5 OEM V212 v44.66.6 validation FAILED', file=sys.stderr)
    for e in errors: print('  - ' + e, file=sys.stderr)
    sys.exit(1)
print('MW5 OEM V212 v44.66.6 validation OK')
