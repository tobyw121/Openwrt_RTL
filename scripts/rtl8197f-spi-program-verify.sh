#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# External-programmer workflow for RTL8197F private full-SPI images.
# The script never writes unless both --write and --yes-i-understand are given.
set -eu

usage() {
	cat <<'USAGE'
Usage:
  rtl8197f-spi-program-verify.sh --board rd05|mw5|ac23 \
    --image FULLFLASH.bin --programmer FLASHROM_PROGRAMMER \
    [--template ORIGINAL_SPI.bin] [--workdir DIR] \
    [--write --yes-i-understand]

Examples:
  # Read-only preflight and two independent chip backups:
  ./scripts/rtl8197f-spi-program-verify.sh --board mw5 \
    --image mw5-fullflash.bin --template mw5-original.bin \
    --programmer ch341a_spi

  # Program, flashrom-verify, full-chip readback, byte compare and layout check:
  ./scripts/rtl8197f-spi-program-verify.sh --board mw5 \
    --image mw5-fullflash.bin --template mw5-original.bin \
    --programmer ch341a_spi --write --yes-i-understand

The programmer string is passed to `flashrom -p` unchanged. The router must be
unpowered and the SPI programmer voltage must match the flash chip. This tool
reduces construction/programming risk; it cannot prove that the board will boot.
USAGE
}

board=""
image=""
template=""
programmer=""
workdir=""
do_write=0
understand=0

while [ "$#" -gt 0 ]; do
	case "$1" in
		--board) board=${2:?missing board}; shift 2 ;;
		--image) image=${2:?missing image}; shift 2 ;;
		--template) template=${2:?missing template}; shift 2 ;;
		--programmer) programmer=${2:?missing programmer}; shift 2 ;;
		--workdir) workdir=${2:?missing workdir}; shift 2 ;;
		--write) do_write=1; shift ;;
		--yes-i-understand) understand=1; shift ;;
		-h|--help) usage; exit 0 ;;
		*) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
	esac
done

case "$board" in rd05|mw5|ac23) ;; *) echo 'ERROR: --board must be rd05, mw5 or ac23' >&2; exit 2 ;; esac
[ -n "$image" ] && [ -f "$image" ] || { echo 'ERROR: --image must name an existing file' >&2; exit 2; }
[ -n "$programmer" ] || { echo 'ERROR: --programmer is required' >&2; exit 2; }
[ -z "$template" ] || [ -f "$template" ] || { echo 'ERROR: --template file not found' >&2; exit 2; }
command -v flashrom >/dev/null 2>&1 || { echo 'ERROR: flashrom is required on the host' >&2; exit 1; }
command -v sha256sum >/dev/null 2>&1 || { echo 'ERROR: sha256sum is required' >&2; exit 1; }
command -v cmp >/dev/null 2>&1 || { echo 'ERROR: cmp is required' >&2; exit 1; }

case "$board" in
	rd05) expected_size=16777216 ;;
	mw5|ac23) expected_size=8388608 ;;
esac
actual_size=$(wc -c < "$image" | tr -d ' ')
[ "$actual_size" -eq "$expected_size" ] || {
	echo "ERROR: image size $actual_size does not match $board size $expected_size" >&2
	exit 1
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
verifier="$script_dir/rtl8197f-fullflash-verify.py"
[ -x "$verifier" ] || { echo "ERROR: verifier not executable: $verifier" >&2; exit 1; }

if [ -n "$template" ]; then
	python3 "$verifier" --board "$board" --template "$template" --image "$image"
fi

if [ -z "$workdir" ]; then
	stamp=$(date +%Y%m%d-%H%M%S)
	workdir="rtl8197f-spi-$board-$stamp"
fi
mkdir -p "$workdir"
workdir=$(CDPATH= cd -- "$workdir" && pwd)
backup1="$workdir/${board}-before-1.bin"
backup2="$workdir/${board}-before-2.bin"
readback="$workdir/${board}-after-readback.bin"
manifest="$workdir/${board}-spi-program-manifest.txt"

{
	echo "board=$board"
	echo "programmer=$programmer"
	echo "image=$image"
	echo "image_size=$actual_size"
	echo "image_sha256=$(sha256sum "$image" | awk '{print $1}')"
	[ -z "$template" ] || echo "template_sha256=$(sha256sum "$template" | awk '{print $1}')"
} > "$manifest"

echo "Reading the physical SPI twice before any write..."
flashrom -p "$programmer" -r "$backup1"
flashrom -p "$programmer" -r "$backup2"
cmp -s "$backup1" "$backup2" || {
	echo 'ERROR: the two pre-write reads differ; check voltage, clip and programmer' >&2
	exit 1
}
backup_size=$(wc -c < "$backup1" | tr -d ' ')
[ "$backup_size" -eq "$expected_size" ] || {
	echo "ERROR: physical chip read size $backup_size does not match expected $expected_size" >&2
	exit 1
}
echo "before_sha256=$(sha256sum "$backup1" | awk '{print $1}')" >> "$manifest"
echo 'before_double_read_identical=true' >> "$manifest"

if [ -n "$template" ]; then
	cmp -s "$backup1" "$template" || {
		echo 'ERROR: current physical chip differs from the template used to personalize the image' >&2
		echo 'Refusing to write. Rebuild the fullflash from the fresh physical backup.' >&2
		exit 1
	}
	echo 'physical_chip_matches_template=true' >> "$manifest"
else
	echo 'WARNING: no --template supplied; protected-region provenance is not proven' >&2
fi

if [ "$do_write" -ne 1 ]; then
	echo "Read-only preflight passed. Backups and manifest: $workdir"
	echo 'No write was requested.'
	exit 0
fi
[ "$understand" -eq 1 ] || {
	echo 'ERROR: writing requires both --write and --yes-i-understand' >&2
	exit 2
}

printf 'Programming %s with %s...\n' "$board" "$programmer"
flashrom -p "$programmer" -w "$image" -v

echo 'Reading the complete SPI after programming...'
flashrom -p "$programmer" -r "$readback"
cmp -s "$readback" "$image" || {
	echo 'ERROR: complete physical readback differs from the requested image' >&2
	exit 1
}
echo "readback_sha256=$(sha256sum "$readback" | awk '{print $1}')" >> "$manifest"
echo 'flashrom_verify_passed=true' >> "$manifest"
echo 'full_chip_readback_identical=true' >> "$manifest"

if [ -n "$template" ]; then
	python3 "$verifier" --board "$board" --template "$template" --image "$readback"
	echo 'layout_and_protected_regions_verified=true' >> "$manifest"
fi

sync
echo "PASS: physical SPI programming and full-chip readback verified"
echo "Manifest: $manifest"
