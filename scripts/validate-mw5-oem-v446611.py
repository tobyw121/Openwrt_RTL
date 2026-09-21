#!/usr/bin/env python3
from pathlib import Path
import sys
R=Path(__file__).resolve().parents[1]
D=R/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'
DTS=R/'target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts'
P330=R/'target/linux/realtek/patches-6.6/330-net-dsa-realtek-mw5-fix-cpu-egress-isolation.patch'
P331=R/'target/linux/realtek/patches-6.6/331-net-dsa-realtek-mw5-v43-15-strip-leaked-cpu-tags.patch'
PM=R/'target/linux/realtek/base-files/usr/sbin/mw5-portmode'
WL=R/'target/linux/realtek/base-files/usr/sbin/mw5-wan-lan'
RA=R/'MW5-LIVE-AUDIT/mw5-router-audit.sh'; PA=R/'MW5-LIVE-AUDIT/mw5-pc-audit-interactive.sh'; AN=R/'MW5-LIVE-AUDIT/mw5-pcap-compare.py'
errs=[]
def txt(p): return p.read_text(errors='replace')
def need(p,s,n):
    if s not in txt(p): errs.append(f'{n}: missing {s!r}')
def forbid(p,s,n):
    if s in txt(p): errs.append(f'{n}: forbidden {s!r}')
need(D,'1.8.33-mw5-oem-v212-v44.66.11','version')
for s in ['#define RTL_RTK_MW5_OEM_TX_RING0\t768','#define RTL_RTK_MW5_OEM_RX_RING0\t900','dma_pool_create("rtl8197f-mw5-tx"','reuse_opts1 & RTL_RTK_DESC_OWN','READ_ONCE(priv->tx_skb[idx])']:
    need(D,s,'DMA/ring invariant')
need(DTS,'realtek,descriptor-dwords = <6>;','6 dword'); need(DTS,'realtek,tx-ring-size = <768>;','TX ring'); need(DTS,'realtek,rx-ring-size = <900>;','RX ring')
need(D,'bool mw5_p0_transparent;','transparent state')
need(D,'priv->mw5_p0_transparent = true;','transparent default')
need(D,'(!mw5 || !READ_ONCE(priv->mw5_p0_transparent))','router-mode gate')
need(D,'txm.port_mask = BIT(0);','P0 physical DP')
need(D,'if (READ_ONCE(priv->mw5_p0_transparent)) {','transparent TX branch')
need(D,'txm.vid_valid = false;','no DVID in transparent TX')
need(D,'mw5_dsa_host_vlan version=v44.66.11 p0_mode=%s p0_router_mode=%u tx_dvid_lan=%u tx_dvid_wan=%u physical_dp=p0','runtime P0 diagnostic')
need(D,'p0-transparent','live transparent command'); need(D,'p0-router-vlan','A/B fallback command')
need(P331,'mw5_diag=v44.66.11','switch diag revision'); need(P331,'cpu_tag_policy=v44.66.11','switch policy revision')
need(P330,'MW5 CPU tag parser v44.66.11','switch parser revision')
need(PM,'dsa_host_link=transparent-p0-no-dvid','portmode host-link')
need(WL,'p0_mode=transparent p0_router_mode=0 tx_dvid_lan=0 tx_dvid_wan=0','WANLAN transparent preflight')
need(RA,'v7.8/v44.66.11','router audit revision'); need(PA,'pc_audit=v7.8-v44.66.11','PC audit revision')
need(AN,'vlan_before_rtl8_leaks','generic VLAN-before-DSA detector'); need(AN,'vlan_before_rtl8_vids','VLAN leak VID detector'); need(AN,'realtek_cpu_tag_on_user_wire','DSA leak detector')
for p in [R/'target/linux/realtek/base-files/etc/init.d/mw5-access',PM]:
    for n in ['echo 0 >/sys/module/tag_rtl4_9/parameters/tx_layout','echo 1 >/sys/module/tag_rtl4_9/parameters/tx_layout','echo 2 >/sys/module/tag_rtl4_9/parameters/tx_layout','echo 3 >/sys/module/tag_rtl4_9/parameters/tx_layout']:
        forbid(p,n,'production must not force legacy tagger')
need(R/'target/linux/realtek/rtl8197f/config-6.6','CONFIG_NET_DSA_TAG_RTL8_4=y','rtl8_4 enabled')
need(R/'target/linux/realtek/base-files/etc/uci-defaults/97-mw5-accel',"flow_offloading_hw='0'",'HW flow offload fail closed')
need(D,'priv->hw_napt = false;','HW NAPT disabled'); need(D,'priv->tx_hwlookup = false;','HWLOOKUP disabled')
need(P330,'RTL8365MB_MW5_VLAN_EXT_CTRL','documented VLAN_EXT_CTRL')
need(P330,'RTL8365MB_MW5_VID0_ACTION, 0','VID0 UNTAG write')
need(P331,'vid0_egress_action=%s','VID0 readback')
WEB=R/'target/linux/realtek/base-files/www/cgi-bin/mw5.cgi'
INIT=R/'target/linux/realtek/base-files/etc/init.d/mw5-webui'
need(WEB,'device_name()','dynamic device identity')
need(WEB,'Router-$(device_suffix)','unique fallback identity')
need(WEB,'Physische Ports','physical port UI')
need(WEB,'Verbundene Geräte','client UI')
for bad in ['Tenda Nova MW5 Control','<title>MW5 Control</title>','<div class="hero-chip">MW5</div>']:
    forbid(WEB,bad,'no hard-coded MW5 branding')
need(INIT,'seed_dynamic_hostname','dynamic hostname seed')
need(RA,'-c 6000','bounded router PCAP')
need(RA,'rm -rf "$d"','streamed snapshot cleanup')
need(PA,'vid0_egress_action=untag','VID0 audit preflight')
if errs:
    print('MW5 OEM V212 v44.66.11 validation FAILED',file=sys.stderr)
    for e in errs: print(' - '+e,file=sys.stderr)
    sys.exit(1)
print('MW5 OEM V212 v44.66.11 validation OK')
