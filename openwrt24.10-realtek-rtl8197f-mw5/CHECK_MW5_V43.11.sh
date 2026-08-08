#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
D="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
T="$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"
N="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-netdiag"
H="$ROOT/target/linux/realtek/base-files/etc/mw5-diag-help"
P="$ROOT/target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh"
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
for f in "$D" "$T" "$N" "$H" "$P" "$I" "$F" "$STD"; do
	check "exists ${f#$ROOT/}" test -f "$f"
done
check "driver version v43.11" contains "$D" '1.4.11-sdk-mw5-fcs-uncached-v43.11'
check "MW5 keeps six-dword descriptors" contains "$D" 'priv->desc_stride = 6 * sizeof(u32)'
check "MW5 DTS enables OEM CRC lengths" contains "$ROOT/target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts" 'realtek,sdk-crc-lengths;'
check "MW5 runtime forces OEM CRC lengths" contains "$D" 'priv->sdk_crc_lengths = true;'
check "TX descriptor adds FCS in SDK mode" contains "$D" 'desc_len = max_t(u32, len + ETH_FCS_LEN, 64)'
check "RX strips descriptor FCS in SDK mode" contains "$D" 'len -= ETH_FCS_LEN;'
check "MW5 descriptor helper uses KSEG1 alias" contains "$D" 'CKSEG1ADDR(lower_32_bits(dma) & 0x1fffffff)'
check "MW5 descriptor helper is board gated" regex "$D" 'of_machine_is_compatible\("tenda,nova-mw5"\).*&&'
check "MW5 only enables TX ring0" contains "$D" 'txringcr |= RTL_RTK_TXRINGCR_TX_RING0_EN;'
check "MW5 explicitly clears TX ring1" contains "$D" 'RTL_RTK_TXRINGCR_TX_RING1_EN |'
check "MW5 TX omits EOR in tail-aware mode" contains "$D" 'OEM New_swNic does not set TX EOR'
check "RX length mask follows SDK bits 13:0" contains "$D" 'RTL_RTK_RX_LEN_MASK'
check "RX dp_ext follows SDK bits 23:20" contains "$D" 'RTL_RTK_RX_DP_EXT_MASK'
check "RX extspa follows SDK bits 25:24" contains "$D" 'RTL_RTK_RX_EXTSPA_MASK'
check "RX spa follows SDK bits 15:13" contains "$D" 'RTL_RTK_RX_SPA_MASK'
check "RX has validated CDP helper" contains "$D" 'rtl8197f_rtk_rx_cdp_index'
check "TX has validated CDP helper" contains "$D" 'rtl8197f_rtk_tx_cdp_index'
check "RX CDP advanced is completion" contains "$D" 'mw5_rx_cdp_complete++'
check "RX runout completion is counted" contains "$D" 'mw5_rx_runout_complete++'
check "RX current OWN slot is empty" contains "$D" 'mw5_rx_empty_wait++'
check "RX has invalid-CDP OWN fallback" contains "$D" 'mw5_rx_cdp_fallback++'
check "v43.9 strict OWN comment removed" rejects "$D" 'OWN is authoritative for MW5 RX completion'
check "TX CDP cleanup counter exists" contains "$D" 'mw5_tx_cdp_clean++'
check "TX stale OWN cleanup counter exists" contains "$D" 'mw5_tx_cdp_owned_clean++'
check "TX invalid CDP counter exists" contains "$D" 'mw5_tx_cdp_invalid++'
check "TX own fallback counter exists" contains "$D" 'mw5_tx_own_fallback_clean++'
check "TX cleanup stops at hardware CDP" contains "$D" 'if (idx == hw_idx)'
check "TX CDP is constrained to queued window" contains "$D" 'if (completed > pending)'
check "MW5 xmit does not trust stale TX OWN" contains "$D" 'software head/tail plus the validated CPUTPDCR0 boundary'
check "MW5 TX cleanup leaves completed descriptor untouched" contains "$D" 'does not rewrite a completed 24-byte descriptor'
check "RX recycle repairs DMA address" contains "$D" 'also repairs an address field if a malformed completion corrupted it'
check "RX malformed snapshot exists" contains "$D" 'mw5 rx-bad v43.11:'
check "RX malformed payload dump exists" contains "$D" 'mw5 rx-bad v43.11 data:'
check "RX descriptor trace v43.11 exists" contains "$D" 'mw5 rxdesc v43.11:'
check "TX submit trace v43.11 exists" contains "$D" 'mw5 tx-submit v43.11:'
check "TX completion trace v43.11 exists" contains "$D" 'mw5 tx-complete v43.11:'
check "proc exposes OEM RX CDP counters" contains "$D" 'cdp_complete=%llu runout_complete=%llu empty_wait=%llu cdp_fallback=%llu'
check "proc exposes OEM TX CDP counters" contains "$D" 'cdp_clean=%llu cdp_owned_clean=%llu cdp_invalid=%llu own_fallback_clean=%llu'
check "proc exposes v43.11 descriptor policy" contains "$D" 'descriptor_policy dwords=%u stride=%u kseg1=%u sdk_crc=%u txringcr=0x%08x'
check "real 0x8899 tag is still passed through" contains "$D" 'mw5_rx_tag_passthrough++'
check "RX fallback only guesses physical 1/3" contains "$D" 'source_port != 1 && source_port != 3'
check "RX fallback synthesises s2c marker" regex "$D" 'tag[[:space:]]*=[[:space:]]*RTL_RTK_MW5_RX_TAG_MARKER'
check "RX tagger marker bit10 retained" contains "$T" '#define RTL4_9_RX_MARKER                BIT(10)'
check "RX tagger source port low six bits" contains "$T" '#define RTL4_9_RX_PORT                  GENMASK(5, 0)'
check "RX dedicated decoder retained" contains "$T" 'rtl4_9_decode_rx_tag'
check "RX observed s2c layout retained" contains "$T" 's2c-0400'
check "RX protocol9 compatibility retained" contains "$T" 'proto9-compat'
check "TX still protocol9" contains "$T" '#define RTL4_9_TX_PROTOCOL_VALUE        0x9'
check "TX destination mask retained" regex "$T" 'FIELD_PREP\(RTL4_9_TX_PORT,[[:space:]]*port_mask\)'
check "tagger logs v43.11" contains "$T" 'rtl4_9 v43.11'
check "netdiag reports v43.11" contains "$N" 'MW5 V43.11 AUTOMATIC WIRED TEST'
check "netdiag collects v43.11 RX bad snapshots" contains "$N" 'mw5 rx(desc|-bad) v43\.(10|11)'
check "netdiag collects v43.11 TX completion traces" contains "$N" 'mw5 tx-(wire|submit|complete) v43\.(10|11)'
check "autotest waits for netifd LAN" contains "$N" 'AUTOTEST WAIT OPENWRT NETWORK READY'
check "autotest checks LAN via ubus" contains "$N" 'ubus call network.interface.lan status'
check "netdiag retains standard firewall" contains "$N" 'firewall=STANDARD_ACTIVE'
check "netdiag does not flush nftables" rejects "$N" 'nft flush ruleset'
check "netdiag does not disable firewall" rejects "$N" '/etc/init.d/firewall disable'
check "old firewall-disable defaults absent" test ! -e "$OLD_F"
check "standard firewall migration helper executable" test -x "$F"
check "standard firewall has LAN zone" regex "$STD" 'option name[[:space:]]+lan'
check "standard firewall has WAN zone" regex "$STD" 'option name[[:space:]]+wan'
check "help documents OEM RX CDP" contains "$H" 'USE_SWITCH_RX_CDP'
check "help documents OEM TX CDP" contains "$H" 'USE_SWITCH_TX_CDP'
check "profile announces v43.11" contains "$P" 'MW5 v43.11:'
check "netdiag shell syntax" sh -n "$N"
check "firewall migration shell syntax" sh -n "$F"
check "init shell syntax" sh -n "$I"
check "profile shell syntax" sh -n "$P"
printf 'RESULT pass=%d fail=%d\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
