#!/usr/bin/env bash
set -euo pipefail
ARCHIVE=${1:?Usage: $0 sdk_GPL_MR62X.tar.zst OUTDIR}
OUTDIR=${2:?Usage: $0 sdk_GPL_MR62X.tar.zst OUTDIR}
TMP=$(mktemp -d)
tar --zstd -tf "$ARCHIVE" > "$TMP/filelist.txt"
awk '
/^sdk\/openwrt-21\.02\/target\/linux\/rtknet\// {print; next}
/^sdk\/openwrt-21\.02\/target\/linux\/target\// {print; next}
/^sdk\/openwrt-21\.02\/package\/uboot\/realtek\/generic\// {print; next}
/^sdk\/openwrt-21\.02\/package\/base-files\/files\/etc\// {print; next}
/8197F|8367R|8367RB|mips_97f/ {print; next}
' "$TMP/filelist.txt" | sort -u > "$TMP/relevant_paths.txt"
mkdir -p "$OUTDIR"
tar --zstd -xf "$ARCHIVE" -C "$OUTDIR" -T "$TMP/relevant_paths.txt" || true
