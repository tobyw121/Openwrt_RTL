#!/bin/sh
# Remove stale legacy MW5-only WLAN package folder from older development trees.
# Run from the OpenWrt tree root after unpacking v43.35 over an older checkout.
set -eu
ROOT="$(pwd)"
OLD="$ROOT/package/kernel/rtl8192cd-mw5"
if [ -d "$OLD" ]; then
	printf 'removing obsolete legacy driver folder: %s\n' "$OLD"
	rm -rf "$OLD"
else
	printf 'obsolete legacy driver folder not present: %s\n' "$OLD"
fi
if [ -f "$ROOT/.config" ]; then
	TMP="$ROOT/.config.tmp.$$"
	grep -v -E '^(CONFIG_PACKAGE_kmod-rtl8192cd-mw5=y|# CONFIG_PACKAGE_kmod-rtl8192cd-mw5 is not set)$' "$ROOT/.config" > "$TMP" || true
	mv "$TMP" "$ROOT/.config"
	printf 'removed stale CONFIG_PACKAGE_kmod-rtl8192cd-mw5 entries from .config if present\n'
fi
printf 'keep/use: CONFIG_PACKAGE_kmod-rtl8192cd-rtl8197f-mw5=y\n'
