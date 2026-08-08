#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
D="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
T="$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"
N="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-netdiag"
H="$ROOT/target/linux/realtek/base-files/etc/mw5-diag-help"
P="$ROOT/target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh"
DTS="$ROOT/target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts"
F="$ROOT/target/linux/realtek/base-files/etc/uci-defaults/96-mw5-restore-standard-firewall"
STD="$ROOT/package/network/config/firewall/files/firewall.config"
pass=0
fail=0
check() { desc="$1"; shift; if "$@"; then echo "PASS $desc"; pass=$((pass+1)); else echo "FAIL $desc"; fail=$((fail+1)); fi; }
contains() { grep -Fq -- "$2" "$1"; }
regex() { grep -Eq -- "$2" "$1"; }
rejects() { ! grep -Eq -- "$2" "$1"; }
for f in "$D" "$T" "$N" "$H" "$P" "$DTS" "$F" "$STD"; do check "exists ${f#$ROOT/}" test -f "$f"; done
check "driver core intentionally unchanged at v43.11" contains "$D" '1.4.11-sdk-mw5-fcs-uncached-v43.11'
check "six-DWORD descriptor policy retained" contains "$D" 'priv->desc_stride = 6 * sizeof(u32)'
check "SDK CRC/FCS policy retained" contains "$D" 'priv->sdk_crc_lengths = true;'
check "KSEG1 descriptor alias retained" contains "$D" 'CKSEG1ADDR(lower_32_bits(dma) & 0x1fffffff)'
check "single TX ring retained" contains "$D" 'txringcr |= RTL_RTK_TXRINGCR_TX_RING0_EN;'
check "tagger protocol remains 9" contains "$T" '#define RTL4_9_TX_PROTOCOL_VALUE        0x9'
check "TX destination field is low byte" contains "$T" '#define RTL4_9_TX_PORT                  GENMASK(7, 0)'
check "TX has no speculative learn-disable define" rejects "$T" 'RTL4_9_TX_LEARN_DIS'
check "TX has no bit9 injection" rejects "$T" 'BIT\(9\)'
check "TX word is protocol plus port mask" regex "$T" 'tag = FIELD_PREP\(RTL4_9_TX_PROTOCOL, RTL4_9_TX_PROTOCOL_VALUE\) \|'
check "TX appends port mask only" regex "$T" 'FIELD_PREP\(RTL4_9_TX_PORT, port_mask\);'
check "LAN expected tag documented" contains "$T" '0x9002 for LAN/port 1'
check "WAN expected tag documented" contains "$T" '0x9008 for WAN/port 3'
check "tagger logs v43.12 TX" contains "$T" 'rtl4_9 v43.12 TX'
check "tagger logs v43.12 RX" contains "$T" 'rtl4_9 v43.12 RX'
check "observed RX 0x0400 marker retained" contains "$T" '#define RTL4_9_RX_MARKER                BIT(10)'
check "observed RX layout retained" contains "$T" 's2c-0400'
check "RX direct source-port mapping retained" contains "$T" 'dsa_master_find_slave(master, 0, encoded_port)'
check "MW5 DTS still requests four-byte proto9" contains "$DTS" 'realtek,cpu-tag-4bytes-proto9;'
check "LAN stays physical port 1" regex "$DTS" 'port@1'
check "WAN stays physical port 3" regex "$DTS" 'port@3'
check "CPU stays physical port 6" regex "$DTS" 'port@6'
check "netdiag announces v43.12" contains "$N" 'MW5 V43.12 AUTOMATIC WIRED TEST'
check "netdiag prints expected tag words" contains "$N" 'tx-tag-expected=LAN:0x9002 WAN:0x9008 RX-LAN:0x0401'
check "netdiag captures v43.12 tagger logs" contains "$N" 'rtl4_9 v43\.(9|10|11|12)'
check "netdiag preserves standard firewall" contains "$N" 'firewall=STANDARD_ACTIVE'
check "netdiag does not flush nftables" rejects "$N" 'nft flush ruleset'
check "netdiag does not disable firewall" rejects "$N" '/etc/init.d/firewall disable'
check "firewall restore helper remains" test -x "$F"
check "standard LAN firewall zone remains" regex "$STD" 'option name[[:space:]]+lan'
check "standard WAN firewall zone remains" regex "$STD" 'option name[[:space:]]+wan'
check "profile announces v43.12" contains "$P" 'MW5 v43.12:'
check "help announces v43.12" contains "$H" 'v43.12'
check "netdiag shell syntax" sh -n "$N"
check "profile shell syntax" sh -n "$P"
# Deterministic tag-word proof using POSIX shell arithmetic.
lan=$(( (9 << 12) | (1 << 1) ))
wan=$(( (9 << 12) | (1 << 3) ))
check "LAN tag arithmetic is 0x9002" test "$lan" -eq 36866
check "WAN tag arithmetic is 0x9008" test "$wan" -eq 36872
printf 'RESULT pass=%d fail=%d\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
