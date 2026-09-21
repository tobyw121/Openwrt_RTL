#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
errs=[]
def need(text, needle, label):
    if needle not in text: errs.append(f"missing {label}: {needle}")
drv=(root/'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c').read_text()
net=(root/'target/linux/realtek/base-files/etc/uci-defaults/99-mw5-dsa-network').read_text()
pm=(root/'target/linux/realtek/base-files/usr/sbin/mw5-portmode').read_text()
hw=(root/'target/linux/realtek/base-files/usr/sbin/mw5-hwaccel').read_text()
st=(root/'target/linux/realtek/base-files/usr/sbin/mw5-speedtest').read_text()
banner=(root/'target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh').read_text()
for n in ['1.8.6-mw5-rx-own-fence-v44.65.10','rx_cdp_owned_wait','rx_cdp_owned_recovered','for (own_retry = 0; own_retry < 16; own_retry++)','udelay(1)']:
    need(drv,n,'RX OWN fence')
for n in ["set network.loopback='interface'","set network.loopback.device='lo'","set network.loopback.proto='static'","set network.loopback.ipaddr='127.0.0.1'","set network.loopback.netmask='255.0.0.0'"]:
    need(net,n,'first-boot loopback')
for n in ['ensure_loopback_config()','ensure_loopback_live()','repair_loopback()','repair-loopback) repair_loopback','ip link set dev lo up','ip addr replace 127.0.0.1/8 dev lo']:
    need(pm,n,'portmode loopback')
for n in ['MW5 recovery v44.65.10: preserve live WAN','mode="${1:-hot}"','wan_recovery=preserved-live-lease','mw5-portmode repair-loopback','if ! wan_ready && [ "$mode" = full ]','wan_recovery=explicit-full-portmode-rebuild']:
    need(hw,n,'hot recovery')
for n in ['tool=mw5-speedtest-v44.65.10','dns_lan_client_query=','loopback=PASS','dnsmasq_loopback_socket=PASS']:
    need(st,n,'speedtest health')
need(banner,'MW5 v44.65.10 RX-OWN fence + hot recovery baseline','banner')
for n in ['mw5_rx_tag_unclassified','fallback_wan_da','fallback_lan_da','repaired_inplace']:
    need(drv,n,'fail-closed DSA diagnostics retained')
if errs:
    for e in errs: print('ERROR:', e, file=sys.stderr)
    print(f'MW5 v44.65.10 validation FAILED: {len(errs)} errors', file=sys.stderr)
    sys.exit(1)
print('MW5 v44.65.10 RX OWN fence/hot recovery validation OK')
