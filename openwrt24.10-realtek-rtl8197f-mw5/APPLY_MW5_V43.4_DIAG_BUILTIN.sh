#!/bin/sh
# Apply the v43.4 built-in MW5 diagnostic bundle to a v43.3 source tree.
set -eu

ROOT=${1:-.}
PATCH_FILE=${2:-MW5_V43.4_DIAG_BUILTIN.patch}
case "$PATCH_FILE" in
 /*) ;;
 *) PATCH_FILE="$(CDPATH= cd -- "$(dirname -- "$PATCH_FILE")" && pwd)/$(basename -- "$PATCH_FILE")" ;;
esac

cd "$ROOT"
patch --fuzz=0 -p1 < "$PATCH_FILE"

chmod 0755 \
 target/linux/realtek/base-files/usr/sbin/mw5-netdiag \
 target/linux/realtek/base-files/usr/sbin/MW5_V43.3_DIRECT_LAN_DIAG_FIX.sh \
 target/linux/realtek/base-files/usr/sbin/MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh \
 target/linux/realtek/base-files/etc/init.d/mw5-diag \
 target/linux/realtek/base-files/etc/uci-defaults/97-mw5-diag-enable \
 target/linux/realtek/base-files/etc/profile.d/98-mw5-diag-help.sh \
 CHECK_MW5_DIAG_BUILTIN_V43.4.sh \
 APPLY_MW5_V43.4_DIAG_BUILTIN.sh
chmod 0644 target/linux/realtek/base-files/etc/mw5-diag-help

./CHECK_MW5_DIAG_BUILTIN_V43.4.sh .
