#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Resolve this router's private 8 MiB SPI template automatically and build the
# personalized Tenda Nova MW5 fullflash image.
set -euo pipefail

usage() {
    cat <<'USAGE'
Usage:
  rtl8197f-tenda-mw5-auto-fullflash.sh \
    --topdir DIR [--template auto|FILE] \
    --firmware FILE --output FILE [--manifest FILE]

Automatic search order:
  1. Explicit --template FILE / TENDA_MW5_SPI_TEMPLATE
  2. Known MW5 dump names in the OpenWrt tree
  3. Known MW5 dump names next to the OpenWrt tree
  4. rtk-collect-latest*.tar.gz in either directory; mtd0-ALL.bin is extracted

Every candidate is validated by rtl8197f-tenda-mw5-fullflash.py. A foreign,
truncated or structurally invalid dump is never accepted.
USAGE
}

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

TOPDIR=
TEMPLATE=auto
FIRMWARE=
OUTPUT=
MANIFEST=

while [[ $# -gt 0 ]]; do
    case "$1" in
        --topdir)
            [[ $# -ge 2 ]] || die "--topdir requires a value"
            TOPDIR=$2
            shift 2
            ;;
        --template)
            [[ $# -ge 2 ]] || die "--template requires a value"
            TEMPLATE=$2
            shift 2
            ;;
        --firmware)
            [[ $# -ge 2 ]] || die "--firmware requires a value"
            FIRMWARE=$2
            shift 2
            ;;
        --output)
            [[ $# -ge 2 ]] || die "--output requires a value"
            OUTPUT=$2
            shift 2
            ;;
        --manifest)
            [[ $# -ge 2 ]] || die "--manifest requires a value"
            MANIFEST=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "unknown argument: $1"
            ;;
    esac
done

[[ -n "$TOPDIR" ]] || die "--topdir is required"
[[ -n "$FIRMWARE" ]] || die "--firmware is required"
[[ -n "$OUTPUT" ]] || die "--output is required"

TOPDIR=$(readlink -f "$TOPDIR")
[[ -d "$TOPDIR" ]] || die "OpenWrt top directory not found: $TOPDIR"
[[ -f "$FIRMWARE" ]] || die "combined MW5 firmware not found: $FIRMWARE"
FIRMWARE=$(readlink -f "$FIRMWARE")

BUILDER="$TOPDIR/scripts/rtl8197f-tenda-mw5-fullflash.py"
EXTRACTOR="$TOPDIR/scripts/rtl8197f-tenda-mw5-extract-template.sh"
[[ -x "$BUILDER" ]] || die "missing builder: $BUILDER"
[[ -x "$EXTRACTOR" ]] || die "missing collector extractor: $EXTRACTOR"

PRIVATE_DIR="$TOPDIR/.mw5-private"
CACHE_TEMPLATE="$PRIVATE_DIR/tenda-mw5-spi-template.bin"
mkdir -p "$PRIVATE_DIR"
chmod 700 "$PRIVATE_DIR" 2>/dev/null || true

# Do not leak private paths/data through shell globbing or broad recursive scans.
# Only these exact, documented locations and names are considered.
declare -a FILE_CANDIDATES=()
declare -a ARCHIVE_CANDIDATES=()

if [[ -n "$TEMPLATE" && "$TEMPLATE" != auto ]]; then
    FILE_CANDIDATES+=("$TEMPLATE")
fi

FILE_CANDIDATES+=(
    "$CACHE_TEMPLATE"
    "$TOPDIR/tenda-mw5-spi-template.bin"
    "$TOPDIR/tenda_mw5.bin"
    "$TOPDIR/tenda-mw5.bin"
    "$TOPDIR/mw5-original.bin"
    "$TOPDIR/mw5-spi-backup.bin"
    "$TOPDIR/../tenda-mw5-spi-template.bin"
    "$TOPDIR/../tenda_mw5.bin"
    "$TOPDIR/../tenda-mw5.bin"
    "$TOPDIR/../mw5-original.bin"
    "$TOPDIR/../mw5-spi-backup.bin"
)

ARCHIVE_CANDIDATES+=(
    "$TOPDIR/rtk-collect-latest.tar.gz"
    "$TOPDIR/rtk-collect-latest.tar(1).gz"
    "$TOPDIR/../rtk-collect-latest.tar.gz"
    "$TOPDIR/../rtk-collect-latest.tar(1).gz"
)

# The fullflash builder performs the authoritative MW5 validation. To test a
# candidate without replacing the requested output, build into a private temp.
validate_candidate() {
    local candidate=$1
    local test_output test_manifest
    test_output=$(mktemp "$PRIVATE_DIR/.validate-fullflash.XXXXXX.bin")
    test_manifest="$test_output.manifest"
    if python3 "$BUILDER" \
        --template "$candidate" \
        --firmware "$FIRMWARE" \
        --output "$test_output" \
        --manifest "$test_manifest" \
        >"$test_output.log" 2>&1; then
        rm -f "$test_output" "$test_manifest" "$test_output.log"
        return 0
    fi
    printf 'MW5 SPI auto: rejected candidate %s\n' "$candidate" >&2
    sed -n '1,12p' "$test_output.log" >&2 || true
    rm -f "$test_output" "$test_manifest" "$test_output.log"
    return 1
}

SELECTED=
SEEN='|'
for candidate in "${FILE_CANDIDATES[@]}"; do
    [[ -n "$candidate" ]] || continue
    if [[ "$candidate" != /* ]]; then
        candidate="$TOPDIR/$candidate"
    fi
    candidate=$(readlink -m "$candidate")
    case "$SEEN" in
        *"|$candidate|"*) continue ;;
    esac
    SEEN+="$candidate|"
    [[ -f "$candidate" ]] || continue
    if validate_candidate "$candidate"; then
        SELECTED=$candidate
        break
    fi
done

if [[ -z "$SELECTED" ]]; then
    for archive in "${ARCHIVE_CANDIDATES[@]}"; do
        archive=$(readlink -m "$archive")
        [[ -f "$archive" ]] || continue
        printf 'MW5 SPI auto: extracting mtd0-ALL.bin from %s\n' "$archive"
        rm -f "$CACHE_TEMPLATE" "$CACHE_TEMPLATE.sha256"
        if "$EXTRACTOR" "$archive" "$CACHE_TEMPLATE"; then
            chmod 600 "$CACHE_TEMPLATE" "$CACHE_TEMPLATE.sha256" 2>/dev/null || true
            if validate_candidate "$CACHE_TEMPLATE"; then
                SELECTED=$CACHE_TEMPLATE
                break
            fi
        fi
        rm -f "$CACHE_TEMPLATE" "$CACHE_TEMPLATE.sha256"
    done
fi

if [[ -z "$SELECTED" ]]; then
    cat >&2 <<EOF_ERROR
ERROR: no valid private Tenda Nova MW5 8 MiB SPI template was found.

Place ONE of these files in the OpenWrt top directory or directly next to it:
  tenda-mw5-spi-template.bin
  tenda_mw5.bin
  tenda-mw5.bin
  mw5-original.bin
  mw5-spi-backup.bin
  rtk-collect-latest.tar.gz
  rtk-collect-latest.tar(1).gz

Then run plain:
  make -j1 V=s

No extra TENDA_MW5_SPI_TEMPLATE=... argument is required. The build is stopped
intentionally because a fullflash image without this exact router's original
Bootloader/CFG/CFM/ENV data would be unsafe.
EOF_ERROR
    exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"
if [[ -n "$MANIFEST" ]]; then
    mkdir -p "$(dirname "$MANIFEST")"
fi

printf 'MW5 SPI auto: using validated private template: %s\n' "$SELECTED"
ARGS=(
    --template "$SELECTED"
    --firmware "$FIRMWARE"
    --output "$OUTPUT"
)
if [[ -n "$MANIFEST" ]]; then
    ARGS+=(--manifest "$MANIFEST")
fi
python3 "$BUILDER" "${ARGS[@]}"

sha256sum "$OUTPUT" >"$OUTPUT.sha256"
printf 'MW5 SPI auto: fullflash ready: %s\n' "$OUTPUT"
printf 'MW5 SPI auto: sha256 file:    %s.sha256\n' "$OUTPUT"
[[ -z "$MANIFEST" ]] || printf 'MW5 SPI auto: manifest:       %s\n' "$MANIFEST"
printf 'MW5 SPI auto: PRIVATE DEVICE IMAGE - DO NOT PUBLISH\n'
