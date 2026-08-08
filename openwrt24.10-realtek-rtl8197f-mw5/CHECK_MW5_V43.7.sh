#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
BASE="$ROOT/target/linux/realtek/base-files"
N="$BASE/usr/sbin/mw5-netdiag"
F="$BASE/etc/uci-defaults/96-mw5-disable-firewall"
I="$BASE/etc/init.d/mw5-diag"
P="$BASE/etc/profile.d/98-mw5-diag-help.sh"
H="$BASE/etc/mw5-diag-help"
pass=0
fail=0
check() {
	desc="$1"; shift
	if "$@"; then echo "PASS $desc"; pass=$((pass + 1));
	else echo "FAIL $desc"; fail=$((fail + 1)); fi
}
contains() { grep -Fq -- "$2" "$1"; }
rejects() { ! grep -Eq -- "$2" "$1"; }
check "mw5-netdiag exists" test -f "$N"
check "mw5-netdiag executable" test -x "$N"
check "no argument defaults to autotest" contains "$N" 'MODE="${1:-autotest}"'
check "autotest command documented" contains "$N" 'mw5-netdiag autotest [15..180]'
check "normal LAN bridge phase" contains "$N" 'normal_lan_phase "$wait_seconds"'
check "LAN automatic phase" contains "$N" 'autotest_phase lan "$wait_seconds"'
check "restored routed WAN DHCP phase" contains "$N" 'routed_wan_phase "$wait_seconds"'
check "WAN automatic phase" contains "$N" 'autotest_phase wan "$wait_seconds"'
check "autotest creates one original backup" contains "$N" 'AUTOTEST_BACKUP="$BACKUP_DIR"'
check "autotest direct phases skip backup overwrite" contains "$N" 'MW5_DIAG_SKIP_BACKUP=1'
check "explicit restore helper" contains "$N" 'restore_from_backup()'
check "autotest restores original backup" contains "$N" 'restore_from_backup "$AUTOTEST_BACKUP"'
check "interrupt cleanup exists" contains "$N" 'autotest_cleanup()'
check "interrupt/exit trap installed" contains "$N" "trap 'autotest_cleanup' 0 1 2 15"
check "router generates traffic itself" contains "$N" 'Generating router-originated ARP/IP traffic'
check "normal bridge counters captured" contains "$N" 'br_rx_delta=$((br_rx_after - br_rx_before))'
check "routed WAN lease is detected from interface address" contains "$N" 'wan_addr="$(ip addr show dev wan'
check "automatic peer discovery uses DHCP leases" contains "$N" '/tmp/dhcp.leases'
check "automatic peer discovery uses neighbours" contains "$N" 'ip neigh show dev "$dev"'
check "RX delta captured" contains "$N" 'rx_delta=$((rx_after - rx_before))'
check "TX delta captured" contains "$N" 'tx_delta=$((tx_after - tx_before))'
check "driver proc state captured" contains "$N" '/proc/rd05-rtknet'
check "firewall disable function" contains "$N" 'disable_firewall()'
check "firewall config intentionally emptied" contains "$N" "cat > /etc/config/firewall <<'EOF'"
check "firewall service stopped" contains "$N" '/etc/init.d/firewall stop'
check "firewall service disabled" contains "$N" '/etc/init.d/firewall disable'
check "nftables ruleset flushed" contains "$N" 'nft flush ruleset'
check "restore deliberately does not restore firewall" contains "$N" 'Firewall is deliberately NOT restored in v43.7.'
check "firstboot firewall disable script exists" test -f "$F"
check "firstboot firewall disable script executable" test -x "$F"
check "firstboot is MW5 guarded" contains "$F" '*tenda,nova-mw5*|*nova-mw5*'
check "firstboot empties firewall config" contains "$F" 'Intentionally empty: firewall4 is disabled'
check "firstboot disables firewall service" contains "$F" '/etc/init.d/firewall disable'
check "firstboot flushes nftables" contains "$F" 'nft flush ruleset'
check "boot service enforces no-firewall" contains "$I" 'mw5-netdiag firewall-off'
check "profile advertises one-command test" contains "$P" "run 'mw5-netdiag' once"
check "help file documents no extra commands" contains "$H" 'No additional router or PC commands'
check "direct lan/wan/auto retained" contains "$N" 'direct [lan|wan|auto]'
check "optional odhcpd guard retained" contains "$N" '[ ! -x /etc/init.d/odhcpd ]'
check "no non-POSIX uppercase expansion" rejects "$N" '\$\{target\^\^\}'
check "mw5-netdiag POSIX shell syntax" sh -n "$N"
check "firstboot firewall script shell syntax" sh -n "$F"
check "boot init shell syntax" sh -n "$I"
check "profile shell syntax" sh -n "$P"
printf '%s\n' "RESULT pass=$pass fail=$fail"
[ "$fail" -eq 0 ]
