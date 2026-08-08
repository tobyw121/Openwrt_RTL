#!/bin/sh
# Static validation for the MW5 v43.4 built-in diagnostic bundle.
set -u

ROOT=${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}
BASE="$ROOT/target/linux/realtek/base-files"
pass=0
fail=0

ok() { echo "PASS: $*"; pass=$((pass + 1)); }
bad() { echo "FAIL: $*" >&2; fail=$((fail + 1)); }

require_file()
{
	if [ -f "$1" ]; then ok "file exists: ${1#$ROOT/}"; else bad "missing: ${1#$ROOT/}"; fi
}

require_exec()
{
	if [ -x "$1" ]; then ok "executable: ${1#$ROOT/}"; else bad "not executable: ${1#$ROOT/}"; fi
}

require_grep()
{
	file=$1
	pattern=$2
	desc=$3
	if grep -Eq "$pattern" "$file" 2>/dev/null; then ok "$desc"; else bad "$desc"; fi
}

PRIMARY="$BASE/usr/sbin/mw5-netdiag"
DIRECT="$BASE/usr/sbin/MW5_V43.3_DIRECT_LAN_DIAG_FIX.sh"
LEGACY="$BASE/usr/sbin/MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh"
OLD="$BASE/usr/bin/mw5-v39-check"
INIT="$BASE/etc/init.d/mw5-diag"
ENABLE="$BASE/etc/uci-defaults/97-mw5-diag-enable"
PROFILE="$BASE/etc/profile.d/98-mw5-diag-help.sh"
HELP="$BASE/etc/mw5-diag-help"

for f in "$PRIMARY" "$DIRECT" "$LEGACY" "$OLD" "$INIT" "$ENABLE" "$PROFILE" "$HELP"; do
	require_file "$f"
done
for f in "$PRIMARY" "$DIRECT" "$LEGACY" "$OLD" "$INIT" "$ENABLE" "$PROFILE"; do
	require_exec "$f"
done

for f in "$PRIMARY" "$DIRECT" "$LEGACY" "$OLD" "$INIT" "$ENABLE" "$PROFILE"; do
	if sh -n "$f"; then ok "shell syntax: ${f#$ROOT/}"; else bad "shell syntax: ${f#$ROOT/}"; fi
done

require_grep "$PRIMARY" 'state \[label\]' 'primary tool documents state mode'
require_grep "$PRIMARY" 'full \[label\]' 'primary tool documents full mode'
require_grep "$PRIMARY" 'watch \[seconds\]' 'primary tool includes traffic-window mode'
require_grep "$PRIMARY" 'configure_direct' 'primary tool includes direct LAN topology test'
require_grep "$PRIMARY" 'configure_bridge' 'primary tool includes forced bridge test'
require_grep "$PRIMARY" 'restore_config' 'primary tool includes restore mode'
require_grep "$PRIMARY" '/tmp/mw5-diag' 'primary tool stores reports in /tmp/mw5-diag'
require_grep "$PRIMARY" 'MW5_V43\.2_RUNTIME_TX_NETWORK_TEST\.sh' 'primary tool retains legacy TX test entry point'
require_grep "$PRIMARY" 'board_ok' 'modifying modes are board guarded'
require_grep "$PRIMARY" 'backup_config' 'modifying modes back up UCI configuration'
require_grep "$PRIMARY" 'LAN_MAC=' 'temporary topology changes preserve LAN MAC'
require_grep "$PRIMARY" 'WAN_MAC=' 'temporary topology changes preserve WAN MAC'
require_grep "$INIT" 'START=99' 'boot capture runs after normal network initialization'
require_grep "$INIT" 'boot-capture-started' 'boot capture is duplicate guarded'
require_grep "$INIT" 'boot-latest\.txt' 'boot capture writes a fixed result path'
require_grep "$ENABLE" 'mw5-diag enable' 'first boot enables the diagnostic service'
require_grep "$ENABLE" 'mw5-diag start' 'first boot starts the diagnostic service immediately'
require_grep "$PROFILE" 'mw5-netdiag' 'serial login advertises the built-in command'

require_grep "$DIRECT" 'exec /usr/sbin/mw5-netdiag "\$@"' \
	'v43.3 compatibility command delegates to maintained mw5-netdiag'
require_grep "$PRIMARY" 'direct \[lan\|wan\|auto\]' \
	'maintained direct test supports LAN, WAN and automatic carrier selection'

if cmp -s "$LEGACY" /mnt/data/MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh 2>/dev/null; then
	ok 'embedded v43.2 runtime script matches supplied script'
else
	if [ -e /mnt/data/MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh ]; then
		bad 'embedded v43.2 runtime script differs from supplied script'
	else
		ok 'external v43.2 comparison skipped'
	fi
fi

printf '\nResult: %s PASS, %s FAIL\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
