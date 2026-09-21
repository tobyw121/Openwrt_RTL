#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
def read(p): return (r/p).read_text(errors='replace')
drv=read('target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c')
hw=read('target/linux/realtek/base-files/usr/sbin/mw5-hwaccel')
speed=read('target/linux/realtek/base-files/usr/sbin/mw5-speedtest')
dts=read('target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts')
uartp=read('target/linux/realtek/patches-6.6/322-serial-8250-dw-add-rtl8197fs-tx-alias.patch')
tag=read('target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c')
fw=read('target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall')
checks={
 'driver version': '1.8.10-mw5-safe-tx-runout-v44.65.14' in drv,
 'coherent rx default on': 'static bool mw5_rx_coherent_bounce = true;' in drv,
 'page pool default off': 'static bool mw5_rx_page_pool;' in drv,
 'tx checksum default off': 'static bool mw5_hw_csum;' in drv and 'static bool mw5_hw_csum = true;' not in drv,
 'rx checksum default off': 'static bool mw5_hw_rx_csum;' in drv,
 'sg default off': 'static bool mw5_hw_sg;' in drv and 'static bool mw5_hw_sg = true;' not in drv,
 'tso default off': 'static bool mw5_hw_tso;' in drv,
 'tx checksum sysfs readonly': 'module_param(mw5_hw_csum, bool, 0444);' in drv,
 'sg sysfs readonly': 'module_param(mw5_hw_sg, bool, 0444);' in drv,
 'mbuf enable bit11': '#define RTL_RTK_IE_MBUF_RUNOUT0\t\tBIT(11)' in drv,
 'mbuf status bit16': '#define RTL_RTK_INT_MBUF_RUNOUT0\t\tBIT(16)' in drv,
 'separate irq enable mask': 'RTL_RTK_INT_ENABLE_MASK' in drv and '~RTL_RTK_INT_MBUF_RUNOUT0' in drv,
 'no per-packet runout helper': 'rtl8197f_rtk_ack_rx_runout' not in drv,
 'irq-only runout ack counter': 'if (status & (RTL_RTK_INT_RX_RUNOUT0 | RTL_RTK_INT_MBUF_RUNOUT0))\n\t\tpriv->rx_runout_acks++;' in drv,
 'no automatic rx resync function': 'rtl8197f_rtk_mw5_rx_resync' not in drv,
 'no resync pending state': 'mw5_rx_resync_pending' not in drv,
 'proc reports auto resync off': 'auto_resync=0' in drv,
 'invalid streak diagnostic retained': 'mw5_rx_invalid_streak_max' in drv,
 'dsa anti reinjection retained': 'v44.62.14 RX drop untagged master frame' in tag,
 'safe profile all tx features off': hw.count('set_features off off off off') >= 4,
 'live tx command': 'tx) proc_feature tx "${2:-off}"; verify ;;' in hw,
 'live sg command': 'sg) proc_feature sg "${2:-off}"; verify ;;' in hw,
 'speedtest version': 'tool=mw5-speedtest-v44.65.14' in speed,
 'flow off default': "flow_offloading='0'" in fw and "flow_offloading_hw='0'" in fw,
 'uart irq9 retained': 'interrupts = <9 1>;' in dts,
 'uart alias retained': 'realtek,rx-alias-offset = <0x24>;' in dts and 'realtek,tx-alias-offset = <0x24>;' in dts,
 'uart 16550 quirk retained': 'DW_UART_QUIRK_RTL8197F' in uartp and 'p->type = PORT_16550A;' in uartp,
}
failed=[k for k,v in checks.items() if not v]
for k,v in checks.items(): print(('PASS ' if v else 'FAIL ')+k)
if failed:
 print(f'FAILED: {len(failed)} checks', file=sys.stderr)
 for k in failed: print(' - '+k,file=sys.stderr)
 sys.exit(1)
print(f'PASS v44.65.14: {len(checks)}/{len(checks)} checks')
