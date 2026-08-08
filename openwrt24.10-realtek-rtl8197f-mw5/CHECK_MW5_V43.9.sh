#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
D="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
T="$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"
N="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-netdiag"
H="$ROOT/target/linux/realtek/base-files/etc/mw5-diag-help"
I="$ROOT/target/linux/realtek/base-files/etc/init.d/mw5-diag"
F="$ROOT/target/linux/realtek/base-files/etc/uci-defaults/96-mw5-restore-standard-firewall"
OLD_F="$ROOT/target/linux/realtek/base-files/etc/uci-defaults/96-mw5-disable-firewall"
STD="$ROOT/package/network/config/firewall/files/firewall.config"
pass=0
fail=0
check() {
	desc="$1"
	shift
	if "$@"; then
		echo "PASS $desc"
		pass=$((pass + 1))
	else
		echo "FAIL $desc"
		fail=$((fail + 1))
	fi
}
contains() { grep -Fq -- "$2" "$1"; }
regex() { grep -Eq -- "$2" "$1"; }
rejects() { ! grep -Eq -- "$2" "$1"; }
for f in "$D" "$T" "$N" "$H" "$I" "$F" "$STD"; do
	check "exists ${f#$ROOT/}" test -f "$f"
done
check "driver version v43.9" contains "$D" '1.4.9-sdk-mw5-rxtag-ring-v43.9'
check "RX tag marker is 0x0400" contains "$D" 'RTL_RTK_MW5_RX_TAG_MARKER'
check "RX descriptor fallback keeps proven source-port field" regex "$D" 'MW5_RX_SPA_SHIFT[[:space:]]+13'
check "RX fallback synthesises switch-to-CPU marker" regex "$D" 'tag[[:space:]]*=[[:space:]]*RTL_RTK_MW5_RX_TAG_MARKER'
check "RX fallback only guesses proven physical ports" contains "$D" 'source_port != 1 && source_port != 3'
check "real 0x8899 tag is passed through" contains "$D" 'mw5_rx_tag_passthrough++'
check "MW5 OWN is authoritative" contains "$D" 'OWN is authoritative for MW5 RX completion'
check "MW5 waits on OWN instead of CDP override" contains "$D" 'mw5_rx_own_wait++'
check "RX recycle repairs DMA address" contains "$D" 'also repairs an address field if a malformed completion corrupted it'
check "RX descriptor trace exists" contains "$D" 'mw5 rxdesc v43.9:'
check "RX address mismatch counter exists" contains "$D" 'mw5_rx_addr_mismatch'
check "TX submit trace exists" contains "$D" 'mw5 tx-submit v43.9:'
check "TX completion trace exists" contains "$D" 'mw5 tx-complete v43.9:'
check "TX post-kick OWN counter exists" contains "$D" 'mw5_tx_own_after_kick'
check "proc exposes RX completion diagnostics" contains "$D" 'own_wait=%llu addr_mismatch=%llu desc_trace=%llu'
check "proc exposes TX completion diagnostics" contains "$D" 'mw5_tx_dma submit=%llu complete=%llu own_after_kick=%llu'
check "MW5 delayed reseed remains disabled" regex "$D" '!of_machine_is_compatible\("tenda,nova-mw5"\)'
check "RX tagger marker is bit10" contains "$T" '#define RTL4_9_RX_MARKER                BIT(10)'
check "RX tagger marker mask covers high six bits" contains "$T" '#define RTL4_9_RX_MARKER_MASK           GENMASK(15, 10)'
check "RX tagger source port is low six bits" contains "$T" '#define RTL4_9_RX_PORT                  GENMASK(5, 0)'
check "RX has dedicated decoder" contains "$T" 'rtl4_9_decode_rx_tag'
check "RX prefers observed s2c layout" contains "$T" 's2c-0400'
check "RX keeps protocol9 compatibility decoder" contains "$T" 'proto9-compat'
check "TX still uses protocol 9" contains "$T" '#define RTL4_9_TX_PROTOCOL_VALUE        0x9'
check "TX still sets disable-learning" contains "$T" '#define RTL4_9_TX_LEARN_DIS             BIT(9)'
check "TX still uses destination mask" regex "$T" 'FIELD_PREP\(RTL4_9_TX_PORT,[[:space:]]*port_mask\)'
check "v43.8 protocol-zero rejection text removed" rejects "$T" 'RX unknown protocol'
check "netdiag is v43.9" contains "$N" 'MW5 V43.9 AUTOMATIC WIRED TEST'
check "autotest waits for netifd LAN" contains "$N" 'AUTOTEST WAIT OPENWRT NETWORK READY'
check "autotest checks LAN up through ubus" contains "$N" 'ubus call network.interface.lan status'
check "netdiag retains standard firewall" contains "$N" 'firewall=STANDARD_ACTIVE'
check "netdiag does not flush nftables" rejects "$N" 'nft flush ruleset'
check "netdiag does not disable firewall" rejects "$N" '/etc/init.d/firewall disable'
check "old firewall-disable defaults absent" test ! -e "$OLD_F"
check "standard-firewall migration helper executable" test -x "$F"
check "standard firewall has LAN zone" regex "$STD" 'option name[[:space:]]+lan'
check "standard firewall has WAN zone" regex "$STD" 'option name[[:space:]]+wan'
check "help documents standard firewall" contains "$H" 'Standard OpenWrt firewall4 configuration is retained.'
check "netdiag shell syntax" sh -n "$N"
check "firewall migration shell syntax" sh -n "$F"
check "init shell syntax" sh -n "$I"
printf 'RESULT pass=%d fail=%d\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
