#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
DRV="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
HW="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-hwaccel"
ST="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-speedtest"

need() { grep -F "$2" "$1" >/dev/null || { echo "missing: $2" >&2; exit 1; }; }
reject() { ! grep -F "$2" "$1" >/dev/null || { echo "forbidden: $2" >&2; exit 1; }; }

need "$DRV" '"1.8.4-mw5-forwarding-recovery-v44.65.8"'
need "$DRV" 'static bool mw5_hw_qos;'
need "$DRV" 'static bool mw5_drop_invalid_untagged;'
need "$DRV" 'v44.65.8 hw-qos stable baseline: P0=1 queue'
need "$DRV" 'rtl8197f_rtk_apply_hw_qos(priv);'
need "$HW" "MW5 recovery v44.65.8: restore classic OpenWrt routing/NAT/DNS"
need "$HW" 'mw5-portmode repair-current'
need "$HW" 'set_qos off'
need "$HW" 'set_invalid_drop off'
need "$HW" 'echo 1 > /proc/sys/net/ipv4/ip_forward'
need "$HW" 'lan_to_wan_rule=PASS'
need "$HW" 'wan_masquerade=PASS'
need "$ST" 'dns_lan=PASS'
need "$ST" 'dns_upstream=PASS'
need "$ST" 'conntrack=${ct}/${ctmax}'
need "$ST" 'lan_to_wan_rule=PASS'
need "$ST" 'wan_masquerade=PASS'
reject "$ST" 'nslookup openwrt.org 127.0.0.1'
sh -n "$HW"
sh -n "$ST"
echo 'MW5 v44.65.8 forwarding/recovery validation OK: 19 checks'
