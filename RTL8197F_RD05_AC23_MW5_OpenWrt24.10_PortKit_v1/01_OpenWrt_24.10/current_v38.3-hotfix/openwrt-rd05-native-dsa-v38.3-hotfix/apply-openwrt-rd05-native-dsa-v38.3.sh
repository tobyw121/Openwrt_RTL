#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$PWD}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PATCH="$SCRIPT_DIR/openwrt-rd05-native-dsa-v38.3.patch"
TARGET="$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate"
BASE_HASH="caca6866e0806f02efedaba2c7cc162d2197512625828550081ff569f74ac497"
NEW_HASH="f026a3e29801e4ac9a822ca56193ef04f846c316c1b48fc1f54558651102bf8a"

if [[ ! -f "$ROOT/rules.mk" || ! -d "$ROOT/target/linux/realtek" ]]; then
  echo "ERROR: not an OpenWrt Realtek tree: $ROOT" >&2
  exit 2
fi
if [[ ! -f "$PATCH" ]]; then
  echo "ERROR: patch not found next to this script: $PATCH" >&2
  exit 2
fi
if [[ ! -f "$TARGET" ]]; then
  echo "ERROR: v38.2 calibration helper is missing: $TARGET" >&2
  exit 2
fi

current_hash="$(sha256sum "$TARGET" | awk '{print $1}')"
if [[ "$current_hash" == "$NEW_HASH" ]] && grep -q '^PKG_RELEASE:=11$' "$ROOT/package/network/config/rtl8367d-compat/Makefile"; then
  echo "RD05 v38.3 is already applied."
  exit 0
fi
if [[ "$current_hash" != "$BASE_HASH" ]]; then
  echo "ERROR: unsupported input tree; expected the unmodified v38.2 helper." >&2
  echo "current:  $current_hash" >&2
  echo "expected: $BASE_HASH" >&2
  exit 3
fi

cd "$ROOT"
git apply --check "$PATCH"
git apply "$PATCH"

new_hash="$(sha256sum "$TARGET" | awk '{print $1}')"
if [[ "$new_hash" != "$NEW_HASH" ]]; then
  echo "ERROR: post-apply helper hash mismatch" >&2
  exit 4
fi

echo "Applied RD05 native DSA v38.3 calibration-safety update."
echo "Rebuild with:"
echo "  make target/linux/clean"
echo "  make -j1 target/linux/compile V=s"
