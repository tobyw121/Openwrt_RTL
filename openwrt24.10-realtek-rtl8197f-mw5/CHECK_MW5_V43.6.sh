#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
D="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
T="$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"
N="$ROOT/target/linux/realtek/base-files/usr/sbin/mw5-netdiag"
pass=0
fail=0
check() {
        desc="$1"; shift
        if "$@"; then echo "PASS $desc"; pass=$((pass + 1));
        else echo "FAIL $desc"; fail=$((fail + 1)); fi
}
contains() { grep -Fq -- "$2" "$1"; }
rejects() { ! grep -Eq -- "$2" "$1"; }
check "driver version v43.6" contains "$D" '1.4.6-sdk-mw5-wanlan-v43.6'
check "custom MAC setter" contains "$D" 'rtl8197f_rtk_set_mac_address'
check "MAC setter is registered" contains "$D" '.ndo_set_mac_address'
check "MAC change reseeds tables" contains "$D" '"mac-change"'
check "seeded MAC is recorded" contains "$D" 'rd05_seeded_mac'
check "readable proc show exists" contains "$D" 'rtl8197f_rtk_rd05_proc_show'
check "proc read uses seq_read" contains "$D" '.proc_read = seq_read'
check "MW5 participates in raw RX trace" contains "$D" 'rtl8197f_rtk_p0_external_switch_board()'
check "tagger reads encapsulated protocol" contains "$T" 'encapsulated_proto'
check "tagger explicitly preserves protocol" contains "$T" \
	'skb->protocol = htons(encapsulated_proto)'
check "tagger RX trace marker" contains "$T" 'rtl4_9 v43.6 RX'
check "tagger TX trace marker" contains "$T" 'rtl4_9 v43.6 TX'
check "fixed-width port mask" contains "$T" 'u32 port_mask;'
check "port mask assigned once" contains "$T" 'port_mask = BIT(dp->index);'
check "FIELD_PREP uses port_mask" contains "$T" 'FIELD_PREP(RTL4_9_PORT, port_mask)'
check "TX trace prints port_mask" contains "$T" 'dp->index, port_mask, encapsulated_proto, skb->len'
check "no BIT directly passed to percent-x" rejects "$T" 'mask=0x%x.*BIT\(dp->index\)'
check "direct auto supported" contains "$N" 'direct [lan|wan|auto]'
check "carrier-selected LAN" contains "$N" '/sys/class/net/lan/carrier'
check "carrier-selected WAN" contains "$N" '/sys/class/net/wan/carrier'
check "odhcpd is optional" contains "$N" '[ ! -x /etc/init.d/odhcpd ]'
check "diagnostic shell syntax" sh -n "$N"
printf '%s
' "RESULT pass=$pass fail=$fail"
[ "$fail" -eq 0 ]
