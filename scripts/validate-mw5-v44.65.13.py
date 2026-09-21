#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
drv = (root / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c').read_text()
dts = (root / 'target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts').read_text()
patch = (root / 'target/linux/realtek/patches-6.6/322-serial-8250-dw-add-rtl8197fs-tx-alias.patch').read_text()
hw = (root / 'target/linux/realtek/base-files/usr/sbin/mw5-hwaccel').read_text()
uart = (root / 'target/linux/realtek/base-files/usr/sbin/mw5-uart').read_text()
uart_init = (root / 'target/linux/realtek/base-files/etc/init.d/mw5-uart').read_text()
firstboot = (root / 'target/linux/realtek/base-files/etc/uci-defaults/95-mw5-webui').read_text()
fw = (root / 'target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall').read_text()

checks = {
    'driver version': '1.8.9-mw5-sdk-runout-uart-v44.65.13' in drv,
    'distinct mbuf enable bit': '#define RTL_RTK_IE_MBUF_RUNOUT0\t\tBIT(11)' in drv,
    'mbuf status bit retained': '#define RTL_RTK_INT_MBUF_RUNOUT0\t\tBIT(16)' in drv,
    'separate enable mask': 'RTL_RTK_INT_ENABLE_MASK' in drv and '~RTL_RTK_INT_MBUF_RUNOUT0' in drv,
    'sdk runout ack': 'rtl8197f_rtk_ack_rx_runout' in drv and 'rx_runout_acks' in drv,
    'rx self heal': 'rtl8197f_rtk_mw5_rx_resync' in drv and 'mw5_rx_resync_pending' in drv,
    'invalid burst threshold': 'mw5_rx_invalid_streak >= 8' in drv,
    'invalid fail closed default': 'static bool mw5_drop_invalid_untagged = true;' in drv,
    'tx checksum default on': 'static bool mw5_hw_csum = true;' in drv,
    'rx checksum default off': 'static bool mw5_hw_rx_csum;' in drv,
    'sg default on': 'static bool mw5_hw_sg = true;' in drv,
    'tso default off': 'static bool mw5_hw_tso;' in drv,
    'napt default off': 'static bool mw5_hw_napt_enable;' in drv,
    'wfo default off': 'static bool mw5_extport_wlan_enable;' in drv,
    'mw5 uart alias rx': 'realtek,rx-alias-offset = <0x24>;' in dts,
    'mw5 uart alias tx': 'realtek,tx-alias-offset = <0x24>;' in dts,
    'mw5 uart irq9': 'interrupts = <9 1>;' in dts,
    'mw5 uart fifo16 dt': 'fifo-size = <16>;' in dts,
    'uart fixed 16550': 'p->type = PORT_16550A;' in patch and 'p->fifosize = 16;' in patch,
    'uart rtl8197f quirk': 'DW_UART_QUIRK_RTL8197F' in patch,
    'uart aliases preserve dlab': 'UART_LCR_DLAB' in patch,
    'uart low rx trigger helper': 'echo 1 > "$p"' in uart and 'rx_trig_bytes' in uart,
    'uart boot service': 'START=06' in uart_init and 'mw5-uart tune' in uart_init,
    'uart service enabled': 'mw5-uart enable' in firstboot,
    'safe profile txcsum+sg': 'set_features on off on off' in hw,
    'software flowoff default off': "flow_offloading='0'" in fw,
    'hardware flowoff default off': "flow_offloading_hw='0'" in fw,
}

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(f"{'PASS' if ok else 'FAIL'} {name}")
if failed:
    print(f"FAILED: {len(failed)} checks", file=sys.stderr)
    sys.exit(1)
print(f"PASS v44.65.13: {len(checks)}/{len(checks)} checks")
