#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./extract_wlan_ethernet_switch_sources.sh ./output_dir
# Optional env vars:
#   AX12_ARCHIVE=/path/to/GPL_AX12v1.tar.zst
#   MR62X_ARCHIVE=/path/to/sdk_GPL_MR62X.tar.zst
#   PATH_LIST_DIR=/path/to/downloaded/path-lists

OUT_DIR="${1:-./wlan_ethernet_switch_sources}"
PATH_LIST_DIR="${PATH_LIST_DIR:-$(pwd)}"
AX12_ARCHIVE="${AX12_ARCHIVE:-GPL_AX12v1.tar.zst}"
MR62X_ARCHIVE="${MR62X_ARCHIVE:-sdk_GPL_MR62X.tar.zst}"

mkdir -p "$OUT_DIR"

echo "Extrahiere GPL_AX12v1 WLAN/Ethernet-Switch-Dateien..."
tar --zstd -xf "$AX12_ARCHIVE" -C "$OUT_DIR" --files-from "$PATH_LIST_DIR/GPL_AX12v1_wlan_ethernet_switch_paths.txt"

echo "Extrahiere sdk_GPL_MR62X WLAN/Ethernet-Switch-Dateien..."
tar --zstd -xf "$MR62X_ARCHIVE" -C "$OUT_DIR" --files-from "$PATH_LIST_DIR/sdk_GPL_MR62X_wlan_ethernet_switch_paths.txt"

echo "Fertig: $OUT_DIR"
