#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Static regression guard for MW5 OEM V212 v44.66.9."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DRV = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
DTS = ROOT / 'target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts'
TAG = ROOT / 'target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c'
FW = ROOT / 'target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall'
ACC = ROOT / 'target/linux/realtek/base-files/etc/uci-defaults/97-mw5-accel'
PATCH328 = ROOT / 'target/linux/realtek/patches-6.6/328-net-dsa-realtek-rtl8365mb-add-mw5-rtl8363-family.patch'
PATCH329 = ROOT / 'target/linux/realtek/patches-6.6/329-net-dsa-realtek-add-rtl4-protocol9-for-mw5.patch'
PATCH330 = ROOT / 'target/linux/realtek/patches-6.6/330-net-dsa-realtek-mw5-fix-cpu-egress-isolation.patch'
PATCH331 = ROOT / 'target/linux/realtek/patches-6.6/331-net-dsa-realtek-mw5-v43-15-strip-leaked-cpu-tags.patch'
ACCESS = ROOT / 'target/linux/realtek/base-files/etc/init.d/mw5-access'
PORTMODE = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-portmode'
WANLAN = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-wan-lan'
NETDIAG = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-netdiag'
STATUS = ROOT / 'target/linux/realtek/base-files/usr/sbin/mw5-access-status'
ROUTER_AUDIT = ROOT / 'MW5-LIVE-AUDIT/mw5-router-audit.sh'
PC_AUDIT = ROOT / 'MW5-LIVE-AUDIT/mw5-pc-audit-interactive.sh'
PCAP_ANALYZER = ROOT / 'MW5-LIVE-AUDIT/mw5-pcap-compare.py'
FEEDS = ROOT / 'scripts/feeds'

errors = []
def load(path): return path.read_text(errors='replace')
def need(path, needle, label):
    if needle not in load(path): errors.append(f'{label}: missing {needle!r} in {path.relative_to(ROOT)}')
def forbid(path, needle, label):
    if needle in load(path): errors.append(f'{label}: forbidden {needle!r} in {path.relative_to(ROOT)}')

# V212 geometry and coherent-DMA ownership invariants stay unchanged.
need(DRV, '#define DRV_VERSION\t\t\t"1.8.31-mw5-oem-v212-v44.66.9"', 'driver version')
need(DRV, '#define RTL_RTK_MW5_OEM_TX_RING0\t768', 'OEM TX ring0')
need(DRV, '#define RTL_RTK_MW5_OEM_RX_RING0\t900', 'OEM RX ring0')
need(DTS, 'realtek,descriptor-dwords = <6>;', '6-DWORD descriptor DTS')
need(DTS, 'realtek,tx-ring-size = <768>;', 'TX ring DTS')
need(DTS, 'realtek,rx-ring-size = <900>;', 'RX ring DTS')
need(DRV, 'dma_pool_create("rtl8197f-mw5-tx"', 'coherent TX pool')
need(DRV, 'reuse_opts1 & RTL_RTK_DESC_OWN', 'coherent OWN reuse guard')
need(DRV, 'READ_ONCE(priv->tx_skb[idx])', 'coherent software reuse guard')
need(DRV, 'if (idx == hw_idx)', 'CDP completion boundary')
need(DRV, 'if (confirm & RTL_RTK_DESC_OWN)', 'OWN completion fence')
forbid(DRV, 'reclaim_lag=8', 'estimated reclaim distance removed')

# v44.66.9: stop forcing the experimental four-byte tag on MW5. The standard
# rtl8365mb path defaults to RTL8_4 and patch330 is gated independently by the
# board-cascade property so CPU-port isolation remains applied.
need(DTS, 'realtek,mw5-cpu-cascade;', 'MW5 cascade board gate')
forbid(DTS, 'realtek,cpu-tag-4bytes-proto9;', 'production must not force rtl4_9')
need(PATCH330, 'rtl8365mb_is_mw5_cpu_cascade', 'MW5 isolation independent of tag protocol')
need(PATCH330, '"realtek,mw5-cpu-cascade"', 'MW5 isolation DT gate')
need(PATCH330, 'format4b=%u format8b=%u', 'CPU format readback')
need(PATCH330, '!(cpu_ctrl & RTL8365MB_MW5_CPU_TAG_FORMAT)', '8-byte format decode')
need(PATCH331, 'mw5_diag=v44.66.9', 'diagnostic revision')
need(PATCH331, 'cpu_tag_policy=v44.66.9', 'CPU tag policy revision')

# Keep rtl4_9 implementation available only as diagnostic code; it must not be
# selected by the MW5 DTS. Standard RTL8_4 remains enabled in target config.
need(PATCH329, 'DSA_TAG_PROTO_RTL4_9', 'legacy diagnostic protocol retained')
need(ROOT / 'target/linux/realtek/rtl8197f/config-6.6', 'CONFIG_NET_DSA_TAG_RTL8_4=y', 'rtl8_4 enabled')

# Master recognizes the upstream 8-byte CPU->switch tag and always transports
# it through the only physical cascade, P0/RGMII.
need(DRV, '#define RTL_RTK_MW5_CPU_TAG8_LEN\t8', '8-byte tag length')
need(DRV, '#define RTL_RTK_MW5_RTL8_PROTO_WORD\t0x0400', 'rtl8_4 protocol word')
need(DRV, 'proto_reason != RTL_RTK_MW5_RTL8_PROTO_WORD', 'rtl8_4 TX recognition')
need(DRV, 'get_unaligned_be16(skb->data + 2 * ETH_ALEN + 6)', 'rtl8_4 port-mask word')
need(DRV, 'txm.port_mask = BIT(0);', 'physical P0 descriptor target')
need(DRV, 'txm.vid = RTL_RTK_MW5_LAN_VID;', 'LAN DVID9 host-link classification')
need(DRV, 'txm.vid = RTL_RTK_MW5_WAN_VID;', 'WAN DVID8 host-link classification')
need(DRV, 'txm.vid_valid = true;', 'DVID valid on recognized DSA destinations')
need(DRV, 'mw5-dsa-vlan9-p0-p1-p8-untag-fid0', 'P0 untag member in internal VID9')
need(DRV, 'mw5-dsa-vlan8-p0-p3-untag-fid1', 'P0 untag member in internal VID8')
need(DRV, 'VID9=P0+P1+P8/untag/FID0 VID8=P0+P3/untag/FID1', 'host-link VLAN membership log')
need(DRV, 'mw5_dsa_host_vlan version=v44.66.9 tx_dvid_lan=9 tx_dvid_wan=8 physical_dp=p0 vlan9_p0_untag=1 vlan8_p0_untag=1', 'host VLAN runtime diagnostic')
need(DRV, 'RTL_RTK_MW5_CPU_TAG8_LEN : 0', 'master MTU allows 8-byte tag')
need(DRV, 'rtl8_lan=%llu rtl8_wan=%llu rtl8_other=%llu normal_dp=p0', 'rtl8 proc counters')
need(DRV, 'synth rtl8_4 source-port=%u', '8-byte RX fallback')
need(DRV, 'repair rtl8_4 source-port=%u', '8-byte RX repair')

# Normal runtime must not write the legacy tag_rtl4_9 module parameter.
for path, label in ((ACCESS, 'boot'), (PORTMODE, 'portmode')):
    forbid(path, 'echo 0 >/sys/module/tag_rtl4_9/parameters/tx_layout', f'{label} must not force rtl4_9')
    forbid(path, 'echo 1 >/sys/module/tag_rtl4_9/parameters/tx_layout', f'{label} must not force rtl4_9 native')
    forbid(path, 'echo 2 >/sys/module/tag_rtl4_9/parameters/tx_layout', f'{label} must not force raw')
    forbid(path, 'echo 3 >/sys/module/tag_rtl4_9/parameters/tx_layout', f'{label} must not force descmeta')
need(PORTMODE, 'dsa_tag=rtl8_4', 'portmode reports rtl8_4')
need(WANLAN, 'format8b=1', 'WAN/LAN test checks 8-byte CPU mode')
need(STATUS, 'dsa_tag_expected=rtl8_4', 'status expects rtl8_4')
need(NETDIAG, 'selected_name=rtl8_4-p0', 'netdiag normal selection')
forbid(NETDIAG, 'tag_layout_set "$selected_layout"', 'netdiag must not mutate legacy tag layout')

# Live audit v7.6 preflights hardware CPU format and the physical VLAN0-before-rtl8 leak signature.
need(ROUTER_AUDIT, 'collector v7.6/v44.66.9', 'router audit revision')
need(ROUTER_AUDIT, 'expected_dsa_tag=rtl8_4', 'router audit expected tag')
need(PC_AUDIT, 'pc_audit=v7.6-v44.66.9', 'PC audit revision')
need(PC_AUDIT, 'expected_dsa_tag=rtl8_4', 'PC audit expected tag')
need(PC_AUDIT, 'format8b=1', 'PC audit hardware tag preflight')
need(PC_AUDIT, 'mode=passive-no-lan', 'no-SSH passive evidence mode')
need(PC_AUDIT, 'SNAPLEN=2048', 'full physical capture')
need(PC_AUDIT, 'ControlMaster=auto', 'SSH multiplexing')
need(PC_AUDIT, 'mw5-pcap-compare.py', 'PCAP comparison')
need(PCAP_ANALYZER, 'exact_cross_port_duplicates', 'cross-port duplicate detector')
need(PCAP_ANALYZER, 'payload_as_header_candidates', 'payload-as-header detector')
need(PCAP_ANALYZER, 'vlan0_before_rtl8_leaks', 'v44.66.8 VLAN0-before-DSA leak detector')
need(PCAP_ANALYZER, 'realtek_cpu_tag_on_user_wire', 'user-wire DSA leak detector')

# Legacy unsafe accelerators remain fail-closed.
need(FW, "flow_offloading_hw='0'", 'hardware flowtable disabled')
need(ACC, "flow_offloading_hw='0'", 'accelerator hardware flowtable disabled')
need(DRV, 'priv->hw_napt = false;', 'legacy HW NAPT disabled')
need(DRV, 'priv->tx_hwlookup = false;', 'legacy HWLOOKUP disabled')

# V212 flow-control values remain internal RTL8197F evidence only.
need(PATCH328, 'No unknown flow-control register is written from V212.', 'flow-control domain guard')
for reg in range(0x121f, 0x1227):
    forbid(PATCH328, f'regmap_write(priv->map, 0x{reg:04x}', f'no guessed external flow-control write 0x{reg:04x}')

# Patch syntax regressions fixed in v44.66.6 stay fixed.
need(PATCH329, '@@ -130,6 +130,15 @@ config NET_DSA_TAG_RTL4_A', 'patch329 Kconfig hunk accounting')
forbid(PATCH329, '@@ -130,6 +130,14 @@ config NET_DSA_TAG_RTL4_A', 'malformed patch329 hunk removed')
need(FEEDS, "git -C '$safepath' config core.fileMode false", 'feed fileMode normalization')

# MIPS format casts remain intact.
for cast in (
    '(unsigned int)((d0 & RTL_RTK_TX_TYPE_MASK) >> RTL_RTK_TX_TYPE_SHIFT)',
    '(unsigned int)((d0 & RTL_RTK_TX_PH_LEN_MASK) >> RTL_RTK_TX_PH_LEN_SHIFT)',
    '(unsigned int)((d2 & RTL_RTK_TX_M_LEN_MASK) >> RTL_RTK_TX_M_LEN_SHIFT)',
    '(unsigned int)(d3 & RTL_RTK_TX_DVID_MASK)',
): need(DRV, cast, 'MIPS-safe descriptor diagnostic cast')

if errors:
    print('MW5 OEM V212 v44.66.9 validation FAILED', file=sys.stderr)
    for e in errors: print('  - ' + e, file=sys.stderr)
    sys.exit(1)
print('MW5 OEM V212 v44.66.9 validation OK')
