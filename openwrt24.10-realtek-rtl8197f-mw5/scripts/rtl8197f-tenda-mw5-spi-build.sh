#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  scripts/rtl8197f-tenda-mw5-spi-build.sh TEMPLATE [FIRMWARE] [OUTPUT]

TEMPLATE must be the complete 8 MiB SPI dump from this exact MW5.
When FIRMWARE is omitted, the newest built MW5 firmware-cr6c.bin under
bin/targets/realtek/rtl8197f/ is selected automatically.
EOF
}

if [[ $# -lt 1 || $# -gt 3 ]]; then
    usage >&2
    exit 2
fi

TOPDIR="$(cd "$(dirname "$0")/.." && pwd)"
TEMPLATE="$(readlink -f "$1")"

if [[ $# -ge 2 ]]; then
    FIRMWARE="$(readlink -f "$2")"
else
    mapfile -t CANDIDATES < <(
        find "$TOPDIR/bin/targets/realtek/rtl8197f" -maxdepth 1 -type f \
            -name '*tenda_nova-mw5*firmware-cr6c.bin' -printf '%T@ %p\n' 2>/dev/null |
            sort -nr | cut -d' ' -f2-
    )
    if [[ ${#CANDIDATES[@]} -eq 0 ]]; then
        echo "ERROR: no built MW5 firmware-cr6c.bin found" >&2
        echo "Build it first with: make -j1 V=s" >&2
        exit 1
    fi
    FIRMWARE="${CANDIDATES[0]}"
fi

if [[ $# -ge 3 ]]; then
    OUTPUT="$3"
else
    OUTDIR="$TOPDIR/bin/targets/realtek/rtl8197f"
    mkdir -p "$OUTDIR"
    OUTPUT="$OUTDIR/openwrt-realtek-rtl8197f-tenda_nova-mw5-squashfs-spi-full.bin"
fi

mkdir -p "$(dirname "$OUTPUT")"
MANIFEST="${OUTPUT}.manifest.txt"

python3 "$TOPDIR/scripts/rtl8197f-tenda-mw5-fullflash.py" \
    --template "$TEMPLATE" \
    --firmware "$FIRMWARE" \
    --output "$OUTPUT" \
    --manifest "$MANIFEST"

sha256sum "$OUTPUT" > "${OUTPUT}.sha256"

printf '\nSPI image ready:\n  %s\n  %s\n  %s\n' \
    "$OUTPUT" "$MANIFEST" "${OUTPUT}.sha256"
printf '\nThe output is personalized and contains private data from TEMPLATE.\n'
