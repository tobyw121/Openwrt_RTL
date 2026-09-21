#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
errs=[]
def read(p): return (r/p).read_text(errors='replace')
def need(txt, needle, where):
    if needle not in txt: errs.append(f'{where}: missing {needle!r}')
def forbid(txt, needle, where):
    if needle in txt: errs.append(f'{where}: forbidden {needle!r}')

drv=read('target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c')
p330=read('target/linux/realtek/patches-6.6/330-net-dsa-realtek-mw5-fix-cpu-egress-isolation.patch')
tag=read('target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c')
hw=read('target/linux/realtek/base-files/usr/sbin/mw5-hwaccel')
speed=read('target/linux/realtek/base-files/usr/sbin/mw5-speedtest')
banner=read('target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh')
doc=read('MW5-V44.65.12-COHERENT-RX-LOAD-STABILITY-FIX.md')

if not any(v in drv for v in ['1.8.8-mw5-coherent-rx-v44.65.12','1.8.9-mw5-sdk-runout-uart-v44.65.13','1.8.10-mw5-safe-tx-runout-v44.65.14']): errs.append('driver version: coherent-RX successor missing')
need(drv,'static bool mw5_rx_coherent_bounce = true;','coherent RX stable default')
need(drv,'dma_alloc_coherent(priv->dev,','coherent RX allocation')
need(drv,'priv->rx_coherent_size','coherent RX arena')
need(drv,'memcpy(skb_put(new_skb, len), src, len);','coherent copy-out')
need(drv,'rtl8197f_rtk_mw5_rx_desc_stable','full descriptor stability fence')
need(drv,'dma-address-mismatch','RX DMA address invariant')
need(drv,'coherent_copies=','coherent RX diagnostics')
need(drv,'desc_unstable=','descriptor stability diagnostics')
need(drv,'cdp_owned_recovered','v44.65.10 OWN fence retained')
need(p330,'parse_mask = cpu->mask & RTL8365MB_CPU_PORT_MASK_MASK;','v44.65.11 CPU-only parser retained')
forbid(p330,'parse_mask = (cpu->mask | user_mask)','old widened parser mask')
need(tag,'v44.62.14 RX drop untagged master frame','anti-XDSA guard retained')
need(hw,'mw5_rx_coherent_bounce','hwaccel backend reporting')
need(hw,'ACTIVE coherent fixed DMA bounce ring','hwaccel coherent verification')
if not any(v in speed for v in ['tool=mw5-speedtest-v44.65.12','tool=mw5-speedtest-v44.65.13','tool=mw5-speedtest-v44.65.14']): errs.append('speedtest version: coherent-RX successor missing')
need(speed,'mw5_rx_coherent_bounce','speedtest backend parameter')
if not any(v in banner for v in ['fixed dma_alloc_coherent bounce ring','coherent RX bounce ring']): errs.append('login banner: coherent RX statement missing')
need(doc,'The six requested evidence packages','archive review')
# Coherent-RX compatibility requires risky/policy-changing accelerators off.
# v44.65.13 intentionally enables only stateless TX checksum + bounded SG.
for needle in ['static bool mw5_rx_page_pool;','static bool mw5_hw_rx_csum;','static bool mw5_hw_tso;','static bool mw5_hw_qos;','static bool mw5_hwlookup_enable;','static bool mw5_hw_napt_enable;','static bool mw5_extport_wlan_enable;']:
    need(drv,needle,'stable risky accelerator defaults')
if errs:
    print(f'MW5 v44.65.12 validation FAILED: {len(errs)} errors',file=sys.stderr)
    for e in errs: print(' -',e,file=sys.stderr)
    sys.exit(1)
print('MW5 v44.65.12 coherent RX load-stability validation OK')
