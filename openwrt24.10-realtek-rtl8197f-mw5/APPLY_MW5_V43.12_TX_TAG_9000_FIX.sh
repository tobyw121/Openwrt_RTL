#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu
ROOT="${1:-.}"
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PATCH="${2:-$SCRIPT_DIR/MW5_V43.12_TX_TAG_9000_FIX.patch}"

[ -f "$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c" ] || {
	echo "error: not an expected OpenWrt RTL8197F tree: $ROOT" >&2
	exit 1
}
[ -f "$PATCH" ] || {
	echo "error: patch not found: $PATCH" >&2
	exit 1
}

patch -d "$ROOT" -p1 --fuzz=0 < "$PATCH"
if [ -f "$ROOT/CHECK_MW5_V43.12.sh" ]; then
	sh "$ROOT/CHECK_MW5_V43.12.sh" "$ROOT"
fi
