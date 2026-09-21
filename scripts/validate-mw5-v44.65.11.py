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
p331=read('target/linux/realtek/patches-6.6/331-net-dsa-realtek-mw5-v43-15-strip-leaked-cpu-tags.patch')
tag=read('target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c')
speed=read('target/linux/realtek/base-files/usr/sbin/mw5-speedtest')
hw=read('target/linux/realtek/base-files/usr/sbin/mw5-hwaccel')
banner=read('target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh')

if not any(v in drv for v in ['1.8.8-mw5-coherent-rx-v44.65.12','1.8.9-mw5-sdk-runout-uart-v44.65.13','1.8.10-mw5-safe-tx-runout-v44.65.14']): errs.append('newer driver retaining parser policy: missing supported version')
need(drv,'cdp_owned_wait','RX OWN fence retained')
need(drv,'cdp_owned_recovered','RX OWN fence retained')
need(drv,'mw5_rx_unclassified_detail','unclassified diagnostics')
if not any(v in drv for v in ['mw5 dsa-rx v44.65.11 unclassified:','mw5 dsa-rx v44.65.13 unclassified:','mw5 dsa-rx v44.65.14 unclassified:']): errs.append('bounded unclassified sample: missing')
need(p330,'parse_mask = cpu->mask & RTL8365MB_CPU_PORT_MASK_MASK;','CPU-only parser mask')
forbid(p330,'parse_mask = (cpu->mask | user_mask)','old widened parser mask')
need(p330,'MW5 CPU tag parser v44.65.11:','switch boot diagnostic')
need(p331,'cpu_tag_policy=v44.65.11','switch proc policy')
need(p331,'expected_cpu_only=0x%x','switch expected mask')
need(p331,'RTL8365MB_CPU_CTRL_INSERTMODE_MASK','insert mode report')
need(tag,'v44.62.14 RX drop untagged master frame','anti-XDSA guard retained')
if not any(v in speed for v in ['tool=mw5-speedtest-v44.65.12','tool=mw5-speedtest-v44.65.13','tool=mw5-speedtest-v44.65.14']): errs.append('newer speedtest retaining parser diagnostics: missing supported version')
need(speed,"cpu_tag_policy=",'speedtest CPU policy output')
need(speed,'mw5_rx_unclassified_detail','speedtest unclassified output')
need(hw,'CPU-tag parser: STABLE CPU6-only','hwaccel verify policy')
if not any(v in banner for v in ['CPU-tag parser remains CPU6-only','CPU-tag parser stays CPU6-only']): errs.append('banner: CPU6-only parser statement missing')
# Parser compatibility requires policy-changing fastpaths to remain off;
# v44.65.13 may enable stateless TX checksum/SG without changing this invariant.
for needle in ['static bool mw5_rx_page_pool;','static bool mw5_hw_tso;','static bool mw5_hw_qos;','static bool mw5_hwlookup_enable;','static bool mw5_hw_napt_enable;','static bool mw5_extport_wlan_enable;']:
    need(drv,needle,'policy-safe defaults')
if errs:
    print(f'MW5 v44.65.11 validation FAILED: {len(errs)} errors',file=sys.stderr)
    for e in errs: print(' -',e,file=sys.stderr)
    sys.exit(1)
print('MW5 v44.65.11 CPU-tag parser compatibility validation OK')
